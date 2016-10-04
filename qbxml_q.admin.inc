<?php
// $Id$

/**
 * @file
 * Admin page callback for the qbxml_q module.
 */

require_once 'qbxml_q.inc';

/**
 * Builds and returns the hrk_custom admin settings form.
 */
function qbxml_q_admin_settings_form() { 
  $form[QBXML_Q_FURI] = array(
      '#title' => t('QBXML_Q incoming and outgoing file directory (recommended to be private)'),
      '#type' => 'textfield',
      '#default_value' => variable_get(QBXML_Q_FURI),
      '#description' => t('Set the directory path for storing QBXML requests and responses.'),
      '#required' => TRUE,
    ); 
  
  return system_settings_form($form);
}



/**
 * Submit function for input_simple_form().

function qbxml_q_admin_settings_form_submit($form, &$form_state)
{
  $furi = get_variable(QBXML_Q_FURI_IN);
  $furi = get_variable(QBXML_Q_FURI_OUT);
  if( !file_prepare_directory($furi, FILE_MODIFY_PERMISSIONS | FILE_CREATE_DIRECTORY) ) {
    set_variable( QBXML_Q_FURI_OUT, "private://" );
  }
}
*/


