<?php
/**
 * Created by PhpStorm.
 * User: Hugh
 * Date: 4/25/2015
 * Time: 9:04 PM
 */

/**
 * Implements hook_rules_action_info()
 * Add a Rules Action for adding something to the QBXML outgoing queue
 */
function qbxml_q_rules_action_info() {
  return array(
    'qbxml_q_action_add_node_to_out_queue' => array(
      'label' => t('Add a QBXML formatted node to the send queue'),
      'group' => t('Beancount'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => 'Node with QBXML request in it',
        ),
        // TODO: implement 'access callback' => , ??
      ),
    ),
    /*
       'qbxml_q_action_add_rendered_field_to_out_queue' => array(
         'label' => t('Add a field rendered as QBXML to the send queue'),
         'group' => t('Quickbooks'),
         'parameter' => array(
           'node' => array(
             'type' => 'node',
             'label' => 'Node with QBXML_field request(s) in it',
           ),
           'field' => array(
             'type' => 'field',
             'label' => 'Field with text of the QBXML request to be rendered'
           ),
         ),
         // TODO: implement 'access callback' => ,
       ),
     */
  );
}


/*
 * Implement the action to take QBXML fields in the node, render them, and put them into the QBXML send Q
 * attach callback information.
 */
function qbxml_q_action_add_node_to_out_queue( $node ) {
  if (!is_object($node) || !isset($node->nid)) {
    watchdog('rules_QBXML_Q', 'Error: object passed is not node. Data: !node',
      array('!node' => print_r($node, TRUE)), WATCHDOG_ERROR);
    return;
  }

  // look for a field named field_qbxml_requestparams and put it in the wrapper (probably just the onError attribute)
  $request_rs_callback = $node->field_qbxml_q_rs_callback[LANGUAGE_NONE][0]['value'];
  $request_rq_params = $node->field_qbxml_q_rq_params[LANGUAGE_NONE][0]['value'];
  /* $node_view = node_view( $node, 'qbxml' );
  $rendered_node = drupal_render( $node_view );
  watchdog( 'qbxml_q', "callback=(" . $request_rs_callback . ')<br>params=(' . $request_rq_params .
    ')<br>rendered=(' . htmlspecialchars($rendered_node), ')' )
  */
  $rendered_node = $node->body[LANGUAGE_NONE][0]['value'];

  $requestlist[] = array( 'qbxml' => $rendered_node, 'callback' => $request_rs_callback );

  _qbxml_q_outgoing_request( $request_rq_params, $requestlist );

}



/*
 * Implement the action to take a text fields in a node, render it as QBXML, and put result into the QBXML send Q
 */
 /*
function qbxml_q_action_add_rendered_field_to_out_queue( $node, $field ) {
  // render field
  if( render field ) {
    $queue = DrupalQueue::get(QBXML_Q_OUT, TRUE);
    $queue->createItem( $qbxml_request );
  }
}
   */

/**
 * Implementation of hook_rules_event_info().
 * @ingroup rules
 */
function qbxml_q_rules_event_info() {
  return array( QBXML_Q_EVENT_IN
     => array(
      'label' => t('A response from Quickbooks can be processed'),
      'module' => 'qbxml_q',
      'arguments' => array(
        'name' => array('type' => 'text', 'label' => t('Name of the response QBXML.')),
        'filename_response' => array('type' => 'text', 'label' => t('Filename where QBXML response is stored.')),
        'callback_json' => array('type' => 'text', 'label' => t('Callback information provided.')),
        'filename_request' => array('type' => 'text', 'label' => t('Filename where original QBXML request is stored.')),
      ),
    ),
  );
}


