<?php

/**
 * @file
 * Handler to map Webform Hidden components to Node fields.
 */

function webformtonode_hidden_map($val, $node, $context) {
  return array('value' => $val);
}