<?php
// $Id$

/*
 * Set the initial send-time for a request
 */
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
function _qbxml_request_set_state( $rid, $state, $bookid=0 ) {
  if( !$bookid ) {
    $num = db_update( 'qbxml' )
      ->fields(array('state' => $state ))
      ->condition( 'rid', $rid )
      ->condition( 'state', array('s', 'i'), IN ) // mark pending operations (sent, iterating, etc.) as state indicated
      ->execute();
  }
  else {
    $num = db_update( 'qbxml' ) // update by bookid only for sent ones, this is for setting error state
      ->fields(array('state' => $state ))
      ->condition( 'bookid', $bookid )
      ->condition( 'state', 's' )
      ->execute();
  }
  return $num;
}

/*
* Implements hook_qbwc_sendRequestXML()
*/
function qbxml_qbwc_sendRequestXML($bookid, $sendRequestXML, $company) {
  $ret = NULL;

  $files = _qbxml_filename($bookid, '(\pN+)', 'q');
  $matchpattern = '/' . basename($files) . '/';
  $outfiles = file_scan_directory(dirname($files), $matchpattern);
  foreach ($outfiles as $absolute => $filename) {
    if (file_exists($absolute)) {
      $handle = fopen($absolute, "r");
      $ret = fread($handle, filesize($absolute));
      fclose($handle);
      file_unmanaged_delete($absolute);
      preg_match($matchpattern, $filename->filename, $matches);
      _qbxml_request_sendtime_set_init($matches[1]);
      _qbxml_request_set_state($matches[1], 's');
      break;
    }
  }
  return $ret;
}

/*
 * Implements a qbxml importer registered with hook_qbxml_importer()
 */
function qbxml_feeds_import( $request, $item ){
  if( !$request ) {
    return( FALSE );
  }
  $bookid = isset($request['@response']['BookID']) ? $request['@response']['BookID'] : 1;
  $requestID = isset($request['@response']['RequestID']) ? $request['@response']['RequestID'] : 1;
  $responsekey = isset($request['@response']['ResponseKey']) ? $request['@response']['ResponseKey'] : '';

  $targetfeedid = isset( $request['@request']['callback']['feedsprefix'] ) ? $request['@request']['callback']['feedsprefix'] : QBXML_FEEDS_PREFIX;
  $targetfeedid .= strtolower($responsekey);
  if( isset(_feeds_importer_digest()[$targetfeedid]) && ($source = feeds_source( $targetfeedid )) ) {
    if( $source->importer->fetcher->hasSourceConfig() ) {
      $class = get_class($source->importer->fetcher);
      if ($class == 'FeedsFileFetcher') {
        $sourceconfig = $source->getConfig();
        $importerconfig = $source->importer->getConfig();
        $filename = _qbxml_filename( $bookid, $requestID, 's' );
        file_unmanaged_save_data($item->asXML(), $filename);
        $sourceconfig[ 'FeedsFileFetcher' ][ 'source' ] = $filename;

        // $source->importer->addConfig( array( 'process_in_background' => FALSE ) ); // let it keep setting from .inc file
        $source->setConfig($sourceconfig);
        if( $importerconfig['process_in_background'] ) {
          variable_set( 'qbxml_feeds_process_limit', variable_get('feeds_process_limit', 50) ); // If uncommented, this shows how to alter the chunk sizes for cron to process in background
          variable_set('feeds_process_limit', 2000);
        } 
        $source->startImport();
        while( !$importerconfig['process_in_background'] && (FEEDS_BATCH_COMPLETE != $source->import()) ); // use this to process if process_in_background not set
        return (TRUE);
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
  if( $qbxml_info = simplexml_load_string($response) ) { // use PHP builtin extension libxml

    foreach ($qbxml_info->QBXMLMsgsRs->children() as $key => $item) {

      $requestID = (int) $item->attributes()->requestID;
      $source = db_select( 'qbxml', 'q' )
        ->condition( 'q.bookid', $bookid)
        ->condition( 'q.rid', $requestID )
        ->fields( 'q' )
        ->execute()
        ->fetchObject();
      if( !$source ) {
        watchdog( 'QPROC_IN', 'Unexpected incoming requestID ' . $requestID . '' );
        continue;
      }
      $groupowner = $source->gid;
      $request = json_decode($source->request, TRUE);
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
          file_unmanaged_save_data( $xmlstr, _qbxml_filename( $bookid, $requestID, 'q' ) );
          $deleterequest = FALSE;
          _qbxml_request_set_state( $requestID, 'i' );
        }
      }

      // send to hook that does generic processing
      if (sizeof(module_implements(HOOK_QBXML_PREPROCESS)) > 0) {
        // Call all modules that implement the hook, and let them make changes to $responsenode.
        if (implode('', module_invoke_all(HOOK_QBXML_PREPROCESS, $request, $item))) {
          qbxml_deleterequest( $bookid, $requestID );
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
      file_unmanaged_save_data($item->asXML(), $filename);
      // send to hook that does generic processing
      if (sizeof(module_implements(HOOK_QBXML_FILEPROCESS)) > 0) {
        // Call all modules that implement the hook, and let them deal with the file
        if (implode('', module_invoke_all(HOOK_QBXML_FILEPROCESS, $filename)) ) {
          if( $deleterequest ) {
            _qbxml_request_delete( $bookid, $requestID );
          }
          file_unmanaged_delete($filename);
          continue;
        }
      }
      // mark as response received but not processed
      _qbxml_request_set_state( $requestID, 'u' );
    }
  }
  $recsleft = db_select( 'qbxml', 'q' )
    ->condition( 'q.gid', $groupowner)
    ->fields( 'q' )
    ->execute()
    ->rowCount();
  $percentdone = (int) 100 / ($recsleft + 1);
  return (int) $percentdone ? (int) $percentdone : (int) 1;
}

/*
 * Implements hook_qbwc_receiveResponseXML().
 */
function qbxml_qbwc_receiveResponseXML($bookid, $response = NULL, $company) {
  // $queue_out = DrupalQueue::get(QBXML_OUT, TRUE);
  $ret = 100;
  if (!$response) {
    watchdog('QBXML', 'ReceiveResponse no response returning 0');
  }
  else {

    if ($response->hresult) {
      // error has happened and is described in hresult and message
      watchdog('QBXML', "Quickbooks error Book#$bookid\n" . print_r($response, true));
      // find open requests for bookid, mark as incomplete and set flag for admin to re-queue individually
      // @todo: set event or flag for admin
      _qbxml_request_set_state( 0, 'e', $bookid );
    }
    else {
      $ret = qbxml_import($bookid, $response->response);
    }
  }
  return ($ret);
}

/*
 * Implements hook_qbwc_closeConnection()
 */
function qbxml_qbwc_closeConnection( $bookid, $response ) {
  _qbxml_request_set_state(0, 'e', $bookid); // request stays in database and is marked as error
  // tod
}
