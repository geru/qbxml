<?php
/**
 * @file
 *
 * These includes enable outside interaction with qbxml_q:
 * * File access
 */

// Filesystem access 
// Incoming and outgoing QBXML requests and responses are stored in this directory.
define( 'QBXML_Q_FURI', 'qbxml_q_furi' ); // URI to use for stashing qbxml requests in and out

// The files are stored with a prefix, the dynamic request id, and a suffix
define( 'QBXML_Q_RQ', '_rq.xml' ); // request format
define( 'QBXML_Q_RS', '_rs.xml' ); // response format
define( 'QBXML_Q_FEEDS_PREFIX', 'qbxml_import_' ); // default prefix for machine name of an importer for response tag (eg: TermsQueryRs) appended to end of name

/* QBXML_Q implements the following hook for any other modules to process incoming QBXML before
 * they are added to the queue.
 * Hooks should return 0 to allow processing to continue, anything else to interrupt processing
 */
define( 'QBXML_Q_HOOK_IN', 'qbxml_q_all_incoming' );
// the hook below simply registers potential incoming processors
define( 'QBXML_Q_HOOK_PROCESSOR', 'qbxml_q_process_incoming' );

// Queues used to sequence incoming and outgoing messages
define( 'QBXML_Q_OUT', 'qbxml_q_out' );
define( 'QBXML_Q_IN', 'qbxml_q_in' );

// Rules events implemented
define( 'QBXML_Q_EVENT_IN', 'qbxml_q_event_incoming_response' );

// Classes implemented
class qbxml_q_incoming {
  public function process( $xmlmsg, $params = NULL ) {
    
  }
}


