<?php

/**
 * Form builder. Configure Webform to node.
 *
 * @ingroup forms
 * @see system_settings_form().
*/

function webformtonode_admin_select_form() {
  
  // Get std object of node types of bundle webform.
  $result = db_query("SELECT nid, title FROM {node} WHERE type = 'webform'")->fetchAll();

  // create array of options for the checkboxes.
  $select = array();
  foreach($result as $values) {
   $select[$values->nid] = check_plain($values->title);
  }

  $form['webformtonode_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select the Webform.'),
    '#options' => $select,
    '#default_value' => config_get('webformtonode.settings', 'webforms'),
    '#description' => t('Select the Webforms which you would like to add the
     Webform to Node functionality.'),
  );

  //backdrop_set_message('<pre>$form - $form<br>'. print_r($form, TRUE) .'</pre>');

  // Add a submit button.
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function webformtonode_admin_select_form_submit($form, $form_state) {

  $config = config('webformtonode.settings');
  $config->set('webforms', $form_state['values']['webformtonode_node_types']);
  $config->save();

  backdrop_set_message(t('The configuration options have been saved.'));
}

// list of webforms where the functionality has been add with the options for
// editing the fields and mapping the field fields from webform to node.
function webformtonode_admin_manage_form() {

  // Get std object of node types of bundle webform.
  // Get a list of webforms created
  $result = db_query("SELECT nid, title FROM {node} WHERE type = 'webform'")->fetchAllAssoc('nid');

//backdrop_set_message('<pre>$result - $result<br>'. print_r($result, TRUE) .'</pre>');

  //Get a list of webforms already saved in config.
  $webform_list = config_get('webformtonode.settings', 'webforms');

  //backdrop_set_message('<pre>webformtonode_admin_manage_forms - $result<br>'. print_r($result, TRUE) .'</pre>');
  //backdrop_set_message('<pre>webformtonode_admin_manage_forms - $webform_list<br>'. print_r($webform_list, TRUE) .'</pre>');
  
  
  // build form to display options for edit etc...
  //----------------------------------------------

  $html_outer_header = '<table> <thead> <tr>Select Webform to edit fields</tr> </thead>';

    $html_outer_row_open = '<tbody>';
    $html_outer_row_close = '</tbody>';

    $html_outer_footer = '</table>';

  $html_render = $html_outer_header . $html_outer_row_open;

  // $webform_list has an array of [nid][value]
  // if value is 0 = not selected
  // if value is not 0 then the webform is selected
  // value will be equal to the [nid] due to the way config selects are saved.
  foreach($webform_list as $key => $value) {

      if ($webform_list[$key] > 0) {

        $title = $result[$key]->title;
        
        //$html_body_row = '<tr><td><a href="/admin/config/webformtonode/fields/list/' . $key . '">' . $title . '</a></td></tr>';
        $html_body_row = '<tr><td><a href="/admin/config/webformtonode/settings/manage_webforms/fields/' . $key . '">' . $title . '</a></td></tr>';

        $html_render = $html_render . $html_body_row;
//        backdrop_set_message('<pre>webformtonode_admin_manage_forms - $title<br>'. print_r($title, TRUE) .'</pre>');
//        backdrop_set_message('<pre>webformtonode_admin_manage_forms - $key<br>'. print_r($key, TRUE) .'</pre>');

      }
   }
   
   $html_render = $html_render . $html_outer_row_close . $html_outer_footer;

   //backdrop_set_message('<pre>webformtonode_admin_manage_forms - $html_render<br>'. print_r($html_render, TRUE) .'</pre>');

   $form['webformtonode_fields_list'] = array(
    '#type' => 'markup',
    '#title' => t('Select the Webform to edit fields.'),
    '#description' => t('Select the Webforms to edit the fields.'),
    '#markup' => $html_render,
    '#pre_render' => array('backdrop_pre_render_markup'),
  );

  return $form;

}


function webformtonode_admin_fields() {

 backdrop_set_message('<pre>webformtonode_admin_fields - $form<br>'. print_r($form, TRUE) .'</pre>');
 $form['webformtonode_fields_list'] = array(
  '#type' => 'markup',
  '#title' => t('Select the Webform to edit fields.'),
  '#description' => t('Select the Webforms to edit the fields.'),
  '#markup' => $html_render,
  '#pre_render' => array('backdrop_pre_render_markup'),
);

return $form;

}

function webformtonode_admin_fields_list_form($form, $form_state, $webformnid) {
  $webformnode = node_load($webformnid);
  

  // Get list of Content Types.
  $nodetypes = node_type_get_types();
  //backdrop_set_message('<pre>webformtonode_admin_fields_list - $nodetypes<br>'. print_r($nodetypes, TRUE) .'</pre>');

  // Populate $select with a list of Content Types
  // [type]= "blog/page/post" etc - [name]=Human readable name.
  $select = array('- Select Content Type -');
  foreach ($nodetypes as $key => $value ) {
      $select[$value->type] = $value->name;
  }
   
   // Build and populate the Content Type Selection list.
   $form['webformtonode_select_content_type'] = array (
    '#type' => 'select',
    '#title' => 'Select content type.',
    '#description' => 'Select the content type to copy this webform to.',
    '#options' => $select,
    '#default_value' => config_get('webformtonode.settings', 'contenttypes'),
  );

  // If content type has been saved, disable widget so it can't be changed.
  // The link needs to be deleted before it can be changed.
  // **to-do** add delete configuration
  if ($form['webformtonode_select_content_type']['#default_value'] > 0 ) {
    //$form['webformtonode_select_content_type']['#disabled'] = TRUE;

    // Build the table showing field mappings. ** move to external function **
    
    // Get list of Webform Components
    foreach ($webformnode->webform[components] as $value) {
      $test[form_key] = $value[form_key];
      $test[cid] = $value[cid];
      $test[name] = $value[name];
      $test[type] = $value[type];
      $test[weight] = $value[weight];
      $test[nid] = 0;
      $test[field] = '';
  
      $component[] = $test;
     }

     $target = field_info_instances('node', 'resources');

     
     backdrop_set_message('<pre>webformtonode_admin_fields_list_form - $target<br>'. print_r($target, TRUE) .'</pre>');
     backdrop_set_message('<pre>webformtonode_admin_fields_list_form - $form<br>'. print_r($form, TRUE) .'</pre>');
  }

   backdrop_set_message('<pre>webformtonode_admin_fields_list_form - $component<br>'. print_r($component, TRUE) .'</pre>');

  // Add a submit button.
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function webformtonode_admin_fields_list_form_submit($form, $form_state) {

  $config = config('webformtonode.settings');
  $config->set('contenttypes', $form_state['values']['webformtonode_select_content_type']);
  $config->save();

  backdrop_set_message(t('The configuration options have been saved.'));
}

function webformtonode_admin_fields_list_form_delete($form, $form_state) {

  $config = config('webformtonode.settings');
  $config->set('contenttypes', $form_state['values']['webformtonode_select_content_type']);
  $config->delete();

  backdrop_set_message(t('The configuration options have been deleted.'));
}



function webformtonode_admin_select_contenttype($form, $form_state, $webformnid) {

  backdrop_set_message('<pre>webformtonode_admin_fields_list_form - $webformnid<br>'. print_r($webformnid, TRUE) .'</pre>');

  backdrop_set_title(t('Select the Content type for this webform'));

  return $form;
}


function webformtonode_admin_select_listfields($form, $form_state, $webformnid) {

  backdrop_set_message('<pre>webformtonode_admin_select_listfields - $webformnid<br>'. print_r($webformnid, TRUE) .'</pre>');

  backdrop_set_title(t('Configure this webform'));

  return $form;
}