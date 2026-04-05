<?php

/**
 * @file
 * Handler to map Webform Textarea components to Node fields.
 */

function webformtonode_textarea_map($val, $node, $context) {
  return array(
    'value' => $val,
    'format' => 'filtered_html',
  );
}