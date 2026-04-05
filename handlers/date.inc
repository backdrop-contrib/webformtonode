<?php

/**
 * @file
 * Handler to map Webform Date components to Node Date fields.
 */

/**
 * Format Webform date array into a Node-compatible date string.
 */
function webformtonode_date_map($val, $node, $context) {
  $item = array();

  // Webform Date comes in as an array: array('year' => 2024, 'month' => 12, 'day' => 05)
  // Check if we have the minimum requirements.
  if (is_array($val) && isset($val['year']) && isset($val['month']) && isset($val['day'])) {

    // Create a standard ISO date string: YYYY-MM-DD
    $date_string = sprintf('%04d-%02d-%02d', $val['year'], $val['month'], $val['day']);

    // Determine the target field type to see if we need time or just the date.
    $field_type = $context['field_info']['type'];

    switch ($field_type) {
      case 'datetime':
      case 'date':
      case 'datestamp':
        // Most Backdrop date fields expect 'value'.
        $item = array('value' => $date_string . ' 00:00:00');
        break;

      default:
        // Fallback for text fields.
        $item = array('value' => $date_string);
        break;
    }
  }

  return $item;
}