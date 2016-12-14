<?php
/**
 * Created by: hugh
 * Date: 12/6/16
 * Time: 11:19 PM
 */

/*
 * Implements hook_feeds_after_import()
 */
function qbxml_feeds_after_import(FeedsSource $source) {
  // watchdog( 'POST_IMPORT', 'source=' . _var_dump2str($source) );
  if (isset($source->exception)) {
    watchdog( 'POST_IMPORT', 'exception=' . var_dump($source->exception) );
    watchdog('QBXML', 'An exception occurred during importing!');
  }
  else {
    $class = get_class($source->importer->fetcher);
    if ($class == 'FeedsFileFetcher') {
      if (strpos($source->id, QBXML_FEEDS_PREFIX) !== FALSE) {
        $filename = $source->getConfig()[ 'FeedsFileFetcher' ][ 'source' ];
        file_unmanaged_delete($filename);
        if( $qbxml_feeds_process_limit =  variable_get('qbxml_feeds_process_limit', 0) )  {
          variable_set('feeds_process_limit', $qbxml_feeds_process_limit );
          variable_del('qbxml_feeds_process_limit');
        }
      }
    }
  }
}

