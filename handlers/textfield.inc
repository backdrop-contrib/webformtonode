<?php

/**
 * @file
 * Handler to map Webform Textfield components to Node fields.
 */

/**
 * Maps Webform textfield values to Node text fields.
 */
function webformtonode_textfield_map($val, $node, $context) {
  // For a textfield, $val is usually a single string.
  // We return the standard Backdrop field array structure.

  $item = array(
    'value' => is_array($val) ? reset($val) : $val,
    'format' => 'filtered_html', // Optional: Ensure a format is set.
  );

  return $item;
}