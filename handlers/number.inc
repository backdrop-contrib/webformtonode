<?php

/**
 * @file
 * Handler to map Webform Number components to Node fields.
 */

/**
 * Maps Webform numeric values to Node numeric or text fields.
 */
function webformtonode_number_map($val, $node, $context) {
  $item = array();

  if ($val === '' || $val === NULL) {
    return $item;
  }

  $field_type = $context['field_info']['type'];

  switch ($field_type) {
    case 'number_integer':
      // Ensure it's a clean integer for the DB.
      $item = array('value' => (int) $val);
      break;

    case 'number_decimal':
    case 'number_float':
      // Ensure it's a float for decimal/float fields.
      $item = array('value' => (float) $val);
      break;

    default:
      // Fallback for Text fields: Keep it as a string.
      $item = array('value' => (string) $val);
      break;
  }

  return $item;
}