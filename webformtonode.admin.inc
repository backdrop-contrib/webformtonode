<?php

/**
 * @file
 * Administration pages and Form API builders for Webform to Node.
 */

/**
 * Form builder: The unified landing page (Command Center).
 */
/**
 * Form builder: The unified landing page (Command Center).
 */
function webformtonode_admin_settings_landing($form, &$form_state) {
  $form = array();

  // Load system menu blocks for the top of the page.
  module_load_include('inc', 'system', 'system.admin');
  $form['menu_list'] = array(
    '#markup' => system_admin_menu_block_page(),
  );

  $config = config('webformtonode.settings');
  $enabled_nids = $config->get('webforms') ?: array();
  $registered_handlers = webformtonode_get_handlers();

  // --- HANDLER FILESYSTEM SCAN (Mismatch Guard) ---
  $path = backdrop_get_path('module', 'webformtonode') . '/handlers';

  /** @var array $files */
  $files = file_scan_directory($path, '/\.inc$/');

  // We check only core handlers for the filesystem mismatch warning.
  $core_count = 0;
  foreach ($registered_handlers as $h) {
    if (isset($h['module']) && $h['module'] == 'webformtonode') {
      $core_count++;
    }
  }

  // Type-hint ensures 'count' is being performed on an actual array.
  $mismatch = (count($files) != $core_count);

  if ($mismatch) {
    backdrop_set_message(t('<strong>Handler Mismatch:</strong> Files on disk do not match the registry. A rebuild is required.'), 'warning');
  }

  // --- WEBFORM STATUS SECTION ---
  $form['status_header'] = array(
    '#markup' => '<h3>' . t('Current Configuration Status') . '</h3>',
  );

  $form['manage_selection_button'] = array(
    '#type' => 'link',
    '#title' => t('Manage webform selection'),
    '#href' => 'admin/config/webformtonode/settings/select_webforms',
    '#attributes' => array(
      'class' => array('button', 'button-secondary'),
    ),
    '#prefix' => '<div style="margin-bottom: 20px;">',
    '#suffix' => '</div>',
  );

  if (!empty($enabled_nids)) {
    $rows = array();
    $nodes = node_load_multiple($enabled_nids);

    foreach ($nodes as $node) {
      $mapping = $config->get('mapping_' . $node->nid . '_fields');
      $status_indicator = !empty($mapping) ? '✅ ' . t('Mapped') : '⚠️ ' . t('Pending');

      $rows[] = array(
        'data' => array(
          '<strong>' . check_plain($node->title) . '</strong>',
          $status_indicator,
          l(t('Configure'), 'admin/config/webformtonode/settings/manage_webforms/fields/' . $node->nid, array(
            'attributes' => array('class' => array('button', 'button-secondary')),
          )),
        ),
      );
    }

    $form['status_report'] = array(
      '#theme' => 'table',
      '#header' => array(t('Webform Title'), t('Status'), t('Operations')),
      '#rows' => $rows,
      '#attributes' => array(
        'class' => array('webformtonode-admin-table'),
      ),
    );
  }

  // --- HANDLER MANAGEMENT SECTION ---
  $form['handler_management'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Handler Management'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['handler_management']['scan_button'] = array(
    '#type' => 'link',
    '#title' => t('Rebuild Handler Registry'),
    '#href' => 'admin/config/webformtonode/settings/scan',
    '#attributes' => array(
      'class' => array('button', 'button-secondary'),
    ),
    '#suffix' => '<br /><br />',
  );

  // --- NESTED TABLE FIELDSET ---
  $form['handler_management']['registry_details'] = array(
    '#type' => 'fieldset',
    '#title' => t('View Registered Handlers'),
    '#collapsible' => TRUE,
    '#collapsed' => !$mismatch,
  );

  $h_header = array(t('Module'), t('Machine Name'), t('Label'), t('Source File'));
  $h_rows = array();

  // 1. Process local files for the table.
  foreach ($files as $file) {
    $name = $file->name;
    $is_registered = isset($registered_handlers[$name]);
    $row_class = $is_registered ? array() : array('webformtonode-handler-pending', 'warning');

    $h_rows[] = array(
      'data' => array(
        '<strong>' . ($is_registered ? $registered_handlers[$name]['module'] : '???') . '</strong>',
        '<code>' . $name . '</code>',
        $is_registered ? $registered_handlers[$name]['label'] : '<strong>' . t('PENDING SCAN') . '</strong>',
        '<em>' . $name . '.inc</em>',
      ),
      'class' => $row_class,
    );
  }

  // 2. Process external/hook-based handlers.
  foreach ($registered_handlers as $name => $info) {
    if ($info['module'] != 'webformtonode') {
      $h_rows[] = array(
        'data' => array(
          '<strong>' . $info['module'] . '</strong>',
          '<code>' . $name . '</code>',
          $info['label'],
          '<em>' . $info['file'] . '.inc</em>',
        ),
      );
    }
  }

  if (!empty($h_rows)) {
    $form['handler_management']['registry_details']['table'] = array(
      '#theme' => 'table',
      '#header' => $h_header,
      '#rows' => $h_rows,
      '#attributes' => array(
        'class' => array('webformtonode-admin-table'),
      ),
    );
  }

  return $form;
}

/**
 * Form builder: Select which Webforms are enabled.
 */
function webformtonode_admin_select_form($form, &$form_state) {
  backdrop_set_title(t('Manage webform selection'));

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
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/webformtonode/settings', array(
      'attributes' => array('class' => array('button')),
    )),
  );

  return $form;
}

/**
 * Submit handler for the webform selection form.
 */
function webformtonode_admin_select_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');
  $old_enabled = $config->get('webforms') ?: array();
  $new_enabled = array_values(array_filter($form_state['values']['webforms']));

  $removed = array_diff($old_enabled, $new_enabled);
  if (!empty($removed)) {
    $query = array(
      'removed' => implode(',', $removed),
      'new_list' => implode(',', $new_enabled),
    );
    $form_state['redirect'] = array('admin/config/webformtonode/settings/select_webforms/confirm', array('query' => $query));
  }
  else {
    $config->set('webforms', $new_enabled)->save();
    $form_state['redirect'] = 'admin/config/webformtonode/settings';
  }
}

/**
 * Form builder: Safety confirmation for removing mappings.
 */
function webformtonode_admin_select_confirm_form($form, &$form_state) {
  $removed_nids = explode(',', $_GET['removed'] ?? '');
  $titles = array();

  foreach ($removed_nids as $nid) {
    if ($node = node_load($nid)) {
      $titles[] = '<strong>' . check_plain($node->title) . '</strong>';
    }
  }

  $form['removed_nids'] = array(
    '#type' => 'value',
    '#value' => $removed_nids,
  );

  $form['new_list'] = array(
    '#type' => 'value',
    '#value' => explode(',', $_GET['new_list'] ?? ''),
  );

  $description = t('Permanently delete mappings for:') . '<ul><li>' . implode('</li><li>', $titles) . '</li></ul>';

  return confirm_form($form, t('Confirm removal?'), 'admin/config/webformtonode/settings/select_webforms', $description, t('Delete'), t('Cancel'));
}

/**
 * Submit handler for the mapping removal confirmation.
 */
function webformtonode_admin_select_confirm_form_submit($form, &$form_state) {
  $config = config('webformtonode.settings');

  foreach ($form_state['values']['removed_nids'] as $nid) {
    $config->clear('mapping_' . $nid . '_type')
      ->clear('mapping_' . $nid . '_fields')
      ->clear('node_status_' . $nid);
  }

  $config->set('webforms', $form_state['values']['new_list'])->save();
  $form_state['redirect'] = 'admin/config/webformtonode/settings';
}

/**
 * Form builder: Configure a specific webform's field mapping.
 */
function webformtonode_admin_fields_list_form($form, &$form_state, $webform_nid) {
  $webform_node = node_load($webform_nid);
  $config = config('webformtonode.settings');
  backdrop_set_title(t('Configure: @title', array('@title' => $webform_node->title)));

  $form['back_link'] = array(
    '#markup' => '<div style="margin-bottom: 20px;">' . l(t('« Back to Settings'), 'admin/config/webformtonode/settings') . '</div>',
    '#weight' => -100,
  );

  $form['webform_nid'] = array(
    '#type' => 'value',
    '#value' => $webform_nid,
  );

  $target_type = $config->get('mapping_' . $webform_nid . '_type');

  $form['target_type'] = array(
    '#type' => 'select',
    '#title' => t('Target Content Type'),
    '#options' => array('' => t('- Select -')) + node_type_get_names(),
    '#default_value' => $target_type,
    '#description' => t('Select the content type. Change and save to update field options.'),
  );

  if ($target_type) {
    $targets = webformtonode_get_target_fields($target_type);
    $current_mapping = $config->get('mapping_' . $webform_nid . '_fields') ?: array();

    $form['mapping'] = array(
      '#tree' => TRUE,
      '#theme' => 'table',
      '#header' => array(t('Webform Component'), t('Node Field')),
      '#rows' => array(),
      '#prefix' => '<h3>' . t('Field Mapping') . '</h3>',
    );

    foreach ($webform_node->webform['components'] as $cid => $comp) {
      if ($comp['type'] == 'fieldset') {
        continue;
      }

      $key = $comp['form_key'];
      $form['mapping'][$key] = array(
        '#type' => 'select',
        '#options' => array('' => t('- Skip -')) + $targets,
        '#default_value' => isset($current_mapping[$key]) ? $current_mapping[$key] : '',
      );

      $form['mapping']['#rows'][] = array(
        $comp['name'] . ' <small>(' . $key . ')</small>',
        array('data' => &$form['mapping'][$key]),
      );
    }

    $form['options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Options'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['options']['node_status'] = array(
      '#type' => 'radios',
      '#title' => t('New Node Status'),
      '#options' => array(
        1 => t('Published'),
        0 => t('Unpublished'),
      ),
      '#default_value' => $config->get('node_status_' . $webform_nid) ?? 1,
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );

  return $form;
}

/**
 * Submit handler for the field mapping form.
 */
/**
 * Submit handler for the field mapping form.
 */
function webformtonode_admin_fields_list_form_submit($form, &$form_state) {
  $nid = $form_state['values']['webform_nid'];
  $new_type = $form_state['values']['target_type'];
  $config = config('webformtonode.settings');
  $old_type = $config->get('mapping_' . $nid . '_type');
  $old_mapping = $config->get('mapping_' . $nid . '_fields');

  // If we are changing types and there was already a mapping, go to confirmation.
  if (!empty($old_type) && $new_type != $old_type && !empty($old_mapping)) {
    $redirect_path = "admin/config/webformtonode/settings/manage_webforms/fields/$nid/confirm-change";
    $form_state['redirect'] = array(
      $redirect_path,
      array('query' => array('new_type' => $new_type)),
    );
    return;
  }

  // Save the configuration.
  $config->set('mapping_' . $nid . '_type', $new_type);
  $config->set('mapping_' . $nid . '_fields', $form_state['values']['mapping'] ?? array());
  $config->set('node_status_' . $nid, (int) ($form_state['values']['node_status'] ?? 1));
  $config->save();

  backdrop_set_message(t('Configuration saved.'));

  // STAY ON SCREEN: Redirect back to the same page instead of the dashboard.
  $form_state['redirect'] = 'admin/config/webformtonode/settings/manage_webforms/fields/' . $nid;
}

/**
 * Fallback confirmation form for type change.
 */
function webformtonode_admin_change_type_confirm_form($form, &$form_state, $nid) {
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $nid,
  );

  $form['new_type'] = array(
    '#type' => 'value',
    '#value' => $_GET['new_type'],
  );

  $cancel_path = "admin/config/webformtonode/settings/manage_webforms/fields/$nid";

  return confirm_form($form, t('Confirm Change?'), $cancel_path, t('Mappings will be cleared.'), t('Confirm'), t('Cancel'));
}

/**
 * Submit handler for the type change confirmation.
 */
function webformtonode_admin_change_type_confirm_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  config('webformtonode.settings')
    ->set('mapping_' . $nid . '_type', $form_state['values']['new_type'])
    ->set('mapping_' . $nid . '_fields', array())
    ->save();

  $form_state['redirect'] = "admin/config/webformtonode/settings/manage_webforms/fields/$nid";
}

/**
 * Menu callback to trigger the registry rebuild.
 */
function webformtonode_admin_scan_callback() {
  $count = webformtonode_rebuild_handlers();
  backdrop_set_message(t('Registry rebuilt! Discovered @count handlers from across the system.', array('@count' => $count)));
  backdrop_goto('admin/config/webformtonode/settings');
}