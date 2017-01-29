<?php
// $Id$

/*
 * Set the initial send-time for a request

function _qbxml_request_sendtime_set_init( $rid ) {
  db_update( 'qbxml' ) // the very first time, the gid of the request is not set, so it gets updated
    ->fields(array('gid' => $rid ))
    ->condition( 'rid', $rid )
    ->execute();
  $num = db_update( 'qbxml' )
    ->fields(array('sent' => REQUEST_TIME ))
    ->condition( 'gid', $rid )
    ->execute();
  return $num;
}
*/
/*
 * Update the send-time for a request
 */
function _qbxml_request_sendtime_update( $rid ) {
  $num = db_update( 'qbxml' )
    ->fields(array('sent' => REQUEST_TIME ))
    ->condition( 'rid', $rid )
    ->execute();
  return $num;
}

/*
 * Set the state of a request
 */
function _qbxml_request_set_state( $rid, $state, $bookid=0, $ticket=NULL ) {
  if( !$bookid ) {
    $num = db_update( 'qbxml' )
      ->fields(array('state' => $state ))
      ->condition( 'rid', $rid )
//      ->condition( 'state', array(QBXML_STATE_SENT, QBXML_STATE_ITERATING), 'IN' ) // mark pending operations (sent, iterating, etc.) as state indicated
      ->execute();
  }
  else {
    $num = db_update( 'qbxml' ) // update by bookid and ticket this is for setting error state when connection is closed unexpectedly
      ->fields(array('state' => $state, 'ticket'=>'' ))
      ->condition( 'bookid', $bookid )
      ->condition( 'ticket', $ticket )
      ->condition( 'state', array(QBXML_STATE_SENT, QBXML_STATE_ITERATING ), 'IN' ) // ignore requests that are queued or already done, but unprocessed, or marked as error
      ->execute();
  }
  return $num;
}

/**
 * Implements hook_qbwc_sendRequestXML()
 * @param $bookid
 * @param $sendRequestXML
 * @param $company
 * @return null|string return value is the request to be sent: contents of pre-prepared file
 */
function qbxml_qbwc_sendRequestXML($bookid, $sendRequestXML, $company) {
  $ret = NULL;
  $query = db_select( 'qbxml', 'q' ) // look for requests currently pending for this ticket
    ->fields( 'q' )
    ->condition( 'bookid', $bookid )
    ->condition( 'state', array(QBXML_STATE_ITERATING, QBXML_STATE_QUEUED, QBXML_STATE_REQUEUE), 'IN')
    ->condition( 'ticket', array($sendRequestXML->ticket) )
    ->orderBy( 'q.rid', 'ASC' )
    ->execute();
  if( !$recs = $query->rowCount() ){ // if none found, look for any available requests for the bookid
    $query = db_select( 'qbxml', 'q' )
      ->fields( 'q' )
      ->condition( 'bookid', $bookid )
      ->condition( 'state', array(QBXML_STATE_ITERATING, QBXML_STATE_QUEUED, QBXML_STATE_REQUEUE), 'IN')
      ->condition( 'ticket', NULL, 'IS NULL' )
      ->orderBy( 'q.rid', 'ASC' )
      ->execute();
  }
  $recs = $query->rowCount();
  while( $query->rowCount() ) {
    $result = $query->fetchAssoc();
    $filename = _qbxml_filename( $bookid, $result['rid'], 'q' );
    if( !file_exists($filename) ) {
      _qbxml_request_set_state( $result['rid'], QBXML_STATE_ERROR );
      watchdog('QBXML', t('Request file not found for Request[:rid]',array(':rid' => $result['rid'])));
      continue;
    }
    $handle = fopen($filename, "r");
    $ret = fread($handle, filesize($filename));
    fclose($handle);
    file_unmanaged_delete($filename);
    if( !$result['gid'] ) { // owner of group and all requests getting sent
      db_update( 'qbxml' ) // the very first time, the gid of the parent request is not set. Update all children requests in one fell swoop
      ->fields( array('state' => QBXML_STATE_SENT,
                      'sent' => REQUEST_TIME,
                      'ticket' => $sendRequestXML->ticket ))
        ->condition( 'gid', $result['rid'] )
        ->execute();
      db_update( 'qbxml' ) // now, update the gid and everything else of the parent request
      ->fields( array('state' => QBXML_STATE_SENT,
        'sent' => REQUEST_TIME,
        'ticket' => $sendRequestXML->ticket,
        'gid' => $result['rid'] ))
        ->condition( 'rid', $result['rid'] )
        ->execute();
    }
    else { // update the status of a request
      db_update( 'qbxml' )
        ->fields( array('state' => QBXML_STATE_SENT,
          'sent' => REQUEST_TIME,
          'ticket' => $sendRequestXML->ticket ))
        ->condition( 'rid', $result['rid'] )
        ->execute();
    }
    break;
  }

  return $ret;
}

/*
 * convert an incoming term into a mapped term
 */
function qbxml_importtermmapper( $term ) {
  return( strtolower($term) );
}

/*
 * Implements a qbxml importer registered with hook_qbxml_importer()
 */
function qbxml_feeds_import( &$request, $item ){
  if( !$request ) {
    return( FALSE );
  }
  $bookid = isset($request['@response']['BookID']) ? $request['@response']['BookID'] : 1;
  $requestID = isset($request['@response']['RequestID']) ? $request['@response']['RequestID'] : 1;
  $responsekey = isset($request['@response']['ResponseKey']) ? $request['@response']['ResponseKey'] : '';

  $importtermmapper = (isset($request['@request']['callback']['importtermmapper']) && function_exists($request['@request']['callback']['importtermmapper']))
    ? $request['@request']['callback']['importtermmapper'] : 'qbxml_importtermmapper';
  $feedsprefix = isset( $request['@request']['callback']['feedsprefix'] ) ? $request['@request']['callback']['feedsprefix'] : QBXML_FEEDS_PREFIX;
  $camelarray = qbxml_camelterm_to_array( $responsekey );
  while( $camelarray ) {
    $feedid = $feedsprefix . $importtermmapper(implode($camelarray));
    $lastterm = array_pop($camelarray);
    // stash the last term in the request as an attribute
    // $item['@response']['tag'][] = $lastterm;
    if (isset(_feeds_importer_digest()[$feedid]) && ($source = feeds_source($feedid))) {
      if ($source->importer->fetcher->hasSourceConfig()) {
        $class = get_class($source->importer->fetcher);
        if ($class == 'FeedsFileFetcher') {
          $sourceconfig = $source->getConfig();
          $importerconfig = $source->importer->getConfig();
          $filename = _qbxml_filename($bookid, $requestID, 's');
          $filename = file_unmanaged_save_data($item->asXML(), $filename);
          $sourceconfig['FeedsFileFetcher']['source'] = $filename;

          // $source->importer->addConfig( array( 'process_in_background' => FALSE ) ); // let it keep setting from .inc file
          $source->setConfig($sourceconfig);
          if ($importerconfig['process_in_background']) {
            variable_set('qbxml_feeds_process_limit', variable_get('feeds_process_limit', 50)); // If uncommented, this shows how to alter the chunk sizes for cron to process in background
            variable_set('feeds_process_limit', 2000);
          }
          $source->startImport();
          while (!$importerconfig['process_in_background'] && (FEEDS_BATCH_COMPLETE != $source->import())) {
            ;
          } // use this to process if process_in_background not set
          return (TRUE);
        }
      }
    }
  }
  // Didn't process
  return( FALSE );
}

/*
 * Worker function to process incoming QBXML responses
 * returns percent complete
 */
function qbxml_import( $bookid, $response ) {
  $groupowner = NULL;
  $deleterequest = TRUE;
  if( $qbxml_info = simplexml_load_string($response->response) ) { // use PHP builtin extension libxml

    foreach ($qbxml_info->QBXMLMsgsRs->children() as $key => $item) {

      $requestID = (int) $item->attributes()->requestID;
      $source = db_select( 'qbxml', 'q' )
        ->condition( 'q.bookid', $bookid)
        ->condition( 'q.rid', $requestID )
        ->fields( 'q' )
        ->execute()
        ->fetchObject();
      if( !$source ) {
        watchdog( 'QBXML', 'Unexpected incoming requestID ' . $requestID . '' );
        continue;
      }
      $groupowner = $source->gid;
      $data = json_decode($source->data, TRUE);
      $request = &$data['request'];
      $request['@response'] = array(
        'RequestID' => $requestID,
        'BookID' => $bookid,
        'ResponseKey' => $key,
      );

      if( isset($item->attributes()->iteratorRemainingCount) ) {
        if( $remainingcount = (int) $item->attributes()->iteratorRemainingCount ) {
          $request['@attributes']['requestID'] = $requestID;
          $request['@attributes']['iterator'] = 'Continue';
          $request['@attributes']['iteratorID'] = $item->attributes()->iteratorID;
          $newrequest = array(
            $request['@request']['parent']['key'] => array(
              '@attributes' => $request['@request']['parent']['attributes'],
              $request['@request']['key'] => $request,
            )
          );
          $xmlstr = _qbxml_array_to_xml($newrequest, 'QBXML');
          file_unmanaged_save_data( $xmlstr, _qbxml_filename( $bookid, $requestID, 'q' ), FILE_EXISTS_REPLACE );
          $deleterequest = FALSE;
          _qbxml_request_set_state( $requestID, QBXML_STATE_ITERATING );
        }
      }

      // send to hook that does generic processing
      if (sizeof(module_implements(HOOK_QBXML_PREPROCESS)) > 0) {
        // Call all modules that implement the hook, and let them make changes to $responsenode.
        if (implode('', module_invoke_all(HOOK_QBXML_PREPROCESS, $request, $item))) {
          _qbxml_request_delete( $bookid, $requestID );
          continue; // hook functions should only return something if they have claimed the data.
        }
      }
      $item->addAttribute( 'BookID', $bookid );
      // send to predetermined, targetted processor hook
      $callback = isset($request['@request']['callback']) ? $request['@request']['callback'] : array( 'importer' => 'qbxml_feeds_import' );
      $userfunc = (isset($callback['importer']) ? $callback['importer'] : 'qbxml_feeds_import' );
      $importerlist = module_invoke_all( HOOK_QBXML_IMPORTERS );
      $userfunc = isset( $importerlist[$userfunc] ) ? function_exists($userfunc) ? $userfunc : 'qbxml_feeds_import' : 'qbxml_feeds_import';
      if ($ret = call_user_func($userfunc, $request, $item)) {
        if( $deleterequest ) {
          _qbxml_request_delete( $bookid, $requestID );
        }
        continue;
      }

      $filename = _qbxml_filename( $bookid, $requestID, 's' );
      $request['#response']['bookid'] = $bookid;
      if( $filename = file_unmanaged_save_data($item->asXML(), $filename) ) {
        // send to hook that does generic processing
        if (sizeof(module_implements(HOOK_QBXML_FILEPROCESS)) > 0) {
          // Call all modules that implement the hook, and let them deal with the file
          if (implode('', module_invoke_all(HOOK_QBXML_FILEPROCESS, $filename))) {
            if ($deleterequest) {
              _qbxml_request_delete($bookid, $requestID);
            }
            file_unmanaged_delete($filename);
            continue;
          }
        }
      }
      // mark as response received but not processed
      if( $deleterequest ) { // keep request and mark as unprocessed
        _qbxml_request_set_state($requestID, QBXML_STATE_UNPROCESSED);
      }
    }
  }
  if( !($recspending = db_select( 'qbxml', 'q' )
    ->condition( 'q.bookid', $bookid )
    ->condition( 'q.ticket', $response->ticket )
    ->condition( 'q.state', array( QBXML_STATE_QUEUED, QBXML_STATE_REQUEUE, QBXML_STATE_ITERATING, QBXML_STATE_SENT ), 'IN' )
    ->fields( 'q' )
    ->execute()
    ->rowCount() ) ) {
    $recspending = db_select( 'qbxml', 'q' )
      ->condition( 'q.bookid', $bookid )
      ->condition( 'q.ticket', NULL, 'IS NULL' )
      ->condition( 'q.state', array( QBXML_STATE_QUEUED, QBXML_STATE_REQUEUE, QBXML_STATE_ITERATING, QBXML_STATE_SENT ), 'IN' )
      ->fields( 'q' )
      ->execute()
      ->rowCount();
  }
  $percentdone = (int) (100 / ($recspending + 1));
  $percentdone = $percentdone == 100 ? 99 : $percentdone; // prevent false 100 from rounding
  return (int) ($recspending ? $percentdone : 100);
}

/*
 * Implements hook_qbwc_receiveResponseXML().
 */
function qbxml_qbwc_receiveResponseXML($bookid, $response = NULL, $data) {
  $ret = 100;
  if (!$response) {
    watchdog('QBXML', 'ReceiveResponse no response returning 0');
  }
  else {

    if ($response->hresult) {
      // error has happened and is described in hresult and message
      watchdog('QBXML', "Quickbooks error Book#$bookid\n" . print_r($response, true));
      // @todo: set event or flag for admin
    }
    else {
      $ret = qbxml_import($bookid, $response);
    }
  }
  return ($ret);
}

/*
 * Implements hook_qbwc_closeConnection()
 */
function qbxml_qbwc_closeConnection( $bookid, $closeConnection ) {
  // pending requests are set to get requeued
  _qbxml_request_set_state(0, QBXML_STATE_REQUEUE, $bookid, $closeConnection->ticket); // pending requests get marked to be requeued
}
