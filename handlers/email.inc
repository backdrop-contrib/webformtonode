<?php

/**
 * @file
 * Handler to map Webform Email components to Node fields.
 */

function webformtonode_email_map($val, $node, $context) {
  // If the target field is an actual Email field, use 'email' key.
  if ($context['field_info']['type'] == 'email') {
    return array('email' => $val);
  }

  // Otherwise, fallback to 'value' for text fields.
  return array('value' => $val);
}