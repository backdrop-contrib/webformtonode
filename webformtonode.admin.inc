<?php

/**
 * @file
 * Administration page callbacks for the Webform to Node module.
 */

/**
 * Form builder: Select which Webforms have "Copy to Node" functionality.
 */
function webformtonode_admin_select_form($form, &$form_state) {
  // Get all nodes of type 'webform'.
  $query = db_select('node', 'n')
    ->fields('n', array('nid', 'title'))
    ->condition('type', 'webform')
    ->execute();

  $options = array();
  foreach ($query as $row) {
    $options[$row->nid] = check_plain($row->title);
  }

  $form['webforms'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled Webforms'),
    '#options' => $options,
    '#default_value' => config_get('webformtonode.settings', 'webforms') ?: array(),
    '#description' => t('Select the Webforms that should allow submissions to be converted into nodes.'),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Enabled Webforms'),
  );

  return $form;
}

/**
 * Submit handler for webformtonode_admin_select_form.
 */
function webformtonode_admin_select_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  // Filter out unselected checkboxes (0 values).
  $enabled = array_filter($form_state['values']['webforms']);
  $config->set('webforms', $enabled);
  $config->save();

  backdrop_set_message(t('Enabled webforms updated.'));
}

/**
 * Page callback: Lists enabled webforms and provides links to map them.
 */
function webformtonode_admin_manage_form($form, &$form_state) {
  $enabled_nids = config_get('webformtonode.settings', 'webforms');

  if (empty($enabled_nids)) {
    $form['help'] = array(
      '#markup' => '<p>' . t('No webforms enabled. Please <a href="@url">select webforms</a> first.',
      array('@url' => url('admin/config/webformtonode/settings/select_webforms'))) . '</p>',
    );
    return $form;
  }

  $header = array(t('Webform Title'), t('Operations'));
  $rows = array();

  $nodes = node_load_multiple($enabled_nids);
  foreach ($nodes as $node) {
    $rows[] = array(
      check_plain($node->title),
      l(t('Configure Mapping'), 'admin/config/webformtonode/settings/manage_webforms/fields/' . $node->nid),
    );
  }

  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No enabled webforms found.'),
  );

  return $form;
}

function webformtonode_admin_fields_list_form($form, &$form_state, $webform_nid) {
  $webform_node = node_load($webform_nid);
  $config = config('webformtonode.settings');
  $target_type = $config->get('mapping_' . $webform_nid . '_type');

  $node_types = node_type_get_names();
  $form['target_type'] = array(
    '#type' => 'select',
    '#title' => t('Target Content Type'),
    '#options' => array('' => t('- Select -')) + $node_types,
    '#default_value' => $target_type,
  );

  if ($target_type) {
    $targets = webformtonode_get_target_fields($target_type);
    $current_mapping = $config->get('mapping_' . $webform_nid . '_fields') ?: array();

    // 1. Define the parent container as a table.
    $form['mapping'] = array(
      '#tree' => TRUE,
      '#theme' => 'table',
      '#header' => array(t('Webform Component'), t('Node Field')),
      '#rows' => array(),
    );

    foreach ($webform_node->webform['components'] as $cid => $component) {
      $key = $component['form_key'];

      // 2. Define the actual form element.
      $form['mapping'][$key] = array(
        '#type' => 'select',
        '#options' => array('' => t('- Skip Field -')) + $targets,
        '#default_value' => isset($current_mapping[$key]) ? $current_mapping[$key] : '',
      );

      // 3. Add the row to the parent's #rows array.
      // We pass the element by reference so the Table Theme renders it correctly.
      $form['mapping']['#rows'][] = array(
        $component['name'] . ' <small>(' . $key . ')</small>',
        array('data' => &$form['mapping'][$key]),
      );
    }
  }

  $form['webform_nid'] = array('#type' => 'value', '#value' => $webform_nid);
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save Mapping'));

  return $form;
}

/**
 * AJAX Callback to refresh the mapping table when Content Type changes.
 */
function webformtonode_admin_fields_refresh($form, $form_state) {
  return $form['mapping_wrapper'];
}

function webformtonode_admin_fields_list_form_submit($form, &$form_state) {
  $nid = $form_state['values']['webform_nid'];
  $config = config('webformtonode.settings');

  $config->set('mapping_' . $nid . '_type', $form_state['values']['target_type']);
  $config->set('mapping_' . $nid . '_fields', $form_state['values']['mapping']);
  $config->save();

  backdrop_set_message(t('Configuration saved.'));
}

/**
 * Page callback: The landing page for Webform to Node settings.
 */
function webformtonode_admin_settings_landing() {
  $build = array();

  // 1. The Standard Admin Menu Block (Links: Select, Manage)
  module_load_include('inc', 'system', 'system.admin');
  $build['menu_list'] = array(
    '#markup' => system_admin_menu_block_page(),
  );

  $config = config('webformtonode.settings');
  $enabled_nids = $config->get('webforms') ?: array();

  // 2. Dashboard / Status Section
  if (!empty($enabled_nids)) {
    // Added 'Operations' to the header
    $header = array(t('Enabled Webform'), t('Status'), t('Operations'));
    $rows = array();

    foreach ($enabled_nids as $nid) {
      $node = node_load($nid);
      if ($node) {
        $mapping = $config->get('mapping_' . $nid . '_fields');
        $status_text = !empty($mapping) ? t('✅ Mapped') : t('⚠️ Pending Mapping');

        // Define the quick link path based on your index 6 wildcard
        $mapping_url = 'admin/config/webformtonode/settings/manage_webforms/fields/' . $nid;

        $rows[] = array(
          l($node->title, 'node/' . $nid),
          $status_text,
          l(t('Edit Mapping'), $mapping_url, array('attributes' => array('class' => array('button', 'button-secondary')))),
        );
      }
    }

    $build['status_report'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#prefix' => '<h3>' . t('Current Configuration Status') . '</h3>',
      '#suffix' => '<hr>',
    );
  }
  else {
    $build['status_report'] = array(
      '#markup' => '<div class="messages warning">' . t('No webforms have been enabled yet. Use the links above to start.') . '</div>',
    );
  }

  return $build;
}
