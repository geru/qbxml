<?php

/**
 * Implements hook_rules_action_info()
 * Add a Rules Action for adding something to the QBXML outgoing queue
 */
function qbxml_rules_action_info() {
  return array(
/*    'qbxml_action_add_node_to_out_queue' => array(
      'label' => t('Add a QBXML formatted node to the send queue'),
      'group' => t('Enterprise'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => 'Node with QBXML request in it',
        ),
        // TODO: implement 'access callback' => , ??
      ),
    ),
*/
    'qbxml_action_add_render_array_to_out_queue' => array(
      'label' => t('Add a QBXMLMsgsRq render array to the send queue'),
      'group' => t('Enterprise'),
      'parameter' => array(
        'qbxmlmsgsrq' => array(
          'type' => 'qbxmlmsgsrq',
          'label' => 'QBXMLMsgsRq render array',
        ),
      ),
    ),
  );
}


/*
 * Implement the action to take QBXML fields in the node, render them, and put them into the QBXML send Q
 * attach callback information.

function qbxml_action_add_node_to_out_queue( $node ) {
  if (!is_object($node) || !isset($node->nid)) {
    watchdog('rules_QBXML', 'Error: object passed is not node. Data: !node',
      array('!node' => print_r($node, TRUE)), WATCHDOG_ERROR);
    return;
  }

  // look for a field named field_qbxml_requestparams and put it in the wrapper (probably just the onError attribute)
  $request_rs_callback = $node->field_qbxml_rs_callback[LANGUAGE_NONE][0]['value'];
  $request_rq_params = $node->field_qbxml_rq_params[LANGUAGE_NONE][0]['value'];
  /* $node_view = node_view( $node, 'qbxml' );
  $rendered_node = drupal_render( $node_view );
  watchdog( 'qbxml', "callback=(" . $request_rs_callback . ')<br>params=(' . $request_rq_params .
    ')<br>rendered=(' . htmlspecialchars($rendered_node), ')' )
  * /
  $rendered_node = $node->body[LANGUAGE_NONE][0]['value'];

  $requestlist[] = array( 'qbxml' => $rendered_node, 'callback' => $request_rs_callback );

  _qbxml_outgoing_request( $request_rq_params, $requestlist );

}
*/

/**
 * Implementation of hook_rules_event_info().
 * @ingroup rules
 */
function qbxml_rules_event_info() {
  return array( QBXML_EVENT_IN
     => array(
      'label' => t('A response from Quickbooks can be processed'),
      'module' => 'qbxml',
      'arguments' => array(
        'name' => array('type' => 'text', 'label' => t('Name of the response QBXML.')),
        'filename_response' => array('type' => 'text', 'label' => t('Filename where QBXML response is stored.')),
        'callback_json' => array('type' => 'text', 'label' => t('Callback information provided.')),
        'filename_request' => array('type' => 'text', 'label' => t('Filename where original QBXML request is stored.')),
      ),
    ),
  );
}

