<?php
/**
 * @file
 *
 * These includes enable outside interaction with qbxml_q:
 * * File access
 */

// Filesystem access 
// Incoming and outgoing QBXML requests and responses are stored in this directory.
define( 'QBXML_Q_FURI', 'qbxml_q' ); // URI to use for stashing qbxml requests in and out

// The files are stored with a prefix, the dynamic request id, and a suffix
define( 'QBXML_Q_FEEDS_PREFIX', 'qbxml_import_' ); // default prefix for machine name of an importer for response tag (eg: TermsQueryRs) appended to end of name

/* QBXML_Q implements the following hook for any other modules to process incoming QBXML before
 * they are added to the queue.
 * Hooks should return 0 to allow processing to continue, anything else to interrupt processing
 */
define( 'HOOK_QBXML_Q_PREPROCESS', 'qbxml_q_preprocess' );
define( 'HOOK_QBXML_Q_IMPORT', 'qbxml_q_import' );
define( 'HOOK_QBXML_Q_FILEPROCESS', 'qbxml_q_fileprocess' );

// Rules events implemented
define( 'QBXML_Q_EVENT_IN', 'qbxml_q_event_incoming_response' );


