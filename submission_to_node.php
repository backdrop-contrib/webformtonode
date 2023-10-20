// The main routine

/**
 * Initialize a new class from a webform teaching submission, creating or
 * overwriting as appropriate. If any arguments are invalid, post an error
 * message and return.
 *
 * @param $wid
 *   The node ID of the webform we are initializing from.
 * @param $sid
 *   The submission ID of the particular submission.
 */
function ousa_class_listing_submission_to_node($wid, $sid) {
  $config = config('ousa_class_listing.settings');
  $webform_node = node_load($wid);
  if (empty($webform_node) || $webform_node->type != 'webform') {
    backdrop_set_message(
      t('There is no webform with ID @wid', array('@wid' => $wid)), 'error');
    return;
  }
  $webform = $webform_node->webform;

  $submission = ousa_webforms_get_submission($wid, $sid);
  if (empty($submission)) {
    backdrop_set_message(
      t('There is no submission for webform %title with submission id @sid',
        array('%title' => $webform_node->title, '@sid' => $sid)),
      'error');
    return;
  }
  $keyed_data = $submission->keyed_data;

  // Usually we will create a new class node, but if a class already exists
  // with the same $sid, we overwrite the values of the existing class node.
  $nid = db_query('
    SELECT entity_id AS nid
    FROM {field_data_field_ousa_class_sid}
    WHERE field_ousa_class_sid_value = :sid
    ', array(':sid' => $sid))
    ->fetchField();
  if (!$nid) {
    $node = entity_create('node', array('type' => 'ousa_class'));
    node_object_prepare($node);
    $node->language = 'und';
    $node->status = 1;
    $node->promote = 0;
    $node->comment = 0;
  }
  else {
    $node = node_load($nid);
  }

  // Start filling the node object. First, fields from current event
  // configuration settings.

  $node->field_ousa_class_event['und'] = array(array(
      'value' =>state_get('ousa_class_listing_event'),
    ));
  $node->field_ousa_class_year['und'] = array(array(
      'value' => state_get('ousa_class_listing_year'),
    ));

  // Whether it is assignable.

  $node->field_ousa_class_assignable['und'] = array(array(
      'value' => state_get('ousa_class_listing_assignable'),
    ));

  // Whether it is a broadcast class.
  // We do not set this value from the teaching form question, because for
  // ordinary conventions, broadcast classes are chosen by admins.

  $node->field_ousa_class_broadcast['und'] = array(array(
      'value' => state_get('ousa_class_listing_stream_all'),
    ));

  // Next, fields from the webform submission.

  $node->title = $keyed_data['class_name'][0];

  if ($keyed_data['civicrm_event_id'][0]) {
    $node->field_ousa_class_civicrm_id['und'] = array(array('value' =>
      $keyed_data['civicrm_event_id'][0]));
  }

  if ($keyed_data['civicrm_event_id2'][0]) {
    $node->field_ousa_class_civicrm_id2['und'] = array(array('value' =>
      $keyed_data['civicrm_event_id2'][0]));
  }

  $node->field_ousa_class_level['und'] = array(array(
      'value' => isset($keyed_data['level'][0]) ? $keyed_data['level'][0] : 0,
    ));

  if (isset($keyed_data['creator_name'][0]) & !empty($keyed_data['creator_name'][0])) {
    $node->field_ousa_class_creator['und'] = array(array(
        'value' => $keyed_data['creator_name'][0],
      ));
  }

  $node->field_ousa_class_teacher['und'] = array(array(
      'value' => $keyed_data['civicrm_participant_full_name'][0] ?? '',
    ));

  $node->body['und'] = array(array(
      'value' => $keyed_data['class_description'][0] ?? '',
    ));

  $node->field_ousa_class_supplies['und'] = array(array(
      'value' => $keyed_data['supplies'][0] ?? '',
    ));

  // Advisories are assembled from a series of separate questions and fields.

  $advisory_items = array();
  if (isset($keyed_data['needs_cuts']) && $keyed_data['needs_cuts'][0]) {
    $advisory_items[] = t('Uses cuts.');
  }
  if (isset($keyed_data['needs_glue']) && $keyed_data['needs_glue'][0]) {
    $advisory_items[] = t('Uses glue.');
  }
  if (isset($keyed_data['taught_from_diagrams']) && $keyed_data['taught_from_diagrams'][0]) {
    $advisory_items[] = t('This design will be taught from diagrams.');
  }
  if (isset($keyed_data['not_completed_in_class']) && $keyed_data['not_completed_in_class'][0]) {
    $advisory_items[] = t('This will likely take longer than the completed class time to complete.');
  }
  if (isset($keyed_data['non_square_paper']) && $keyed_data['non_square_paper'][0]) {
    $non_square_paper = $keyed_data['non_square_paper'][0];
    $advisory_items[] = t('This uses non-square paper: @non_square_paper',
      array('@non_square_paper' => $non_square_paper));
  }
  if (isset($keyed_data['precreased_grid']) && $keyed_data['precreased_grid'][0]) {
    $precreased_grid = $keyed_data['precreased_grid'][0];
    $advisory_items[] = t('Please pre-crease a tessellation grid: @precreased_grid',
      array('@precreased_grid' => $precreased_grid));
  }
  if (isset($keyed_data['num_sheets']) && $keyed_data['num_sheets'][0]) {
    $num_sheets = $keyed_data['num_sheets'][0];
    $advisory_items[] = t('This is a modular/composite requiring @num_sheets sheets.',
      array('@num_sheets' => $num_sheets));
  }
  if (isset($keyed_data['details_other']) && $keyed_data['details_other'][0]) {
    $advisory_items[] = check_plain($keyed_data['details_other'][0]);
  }
  if (!empty($advisory_items)) {
    $advisories = theme('item_list', array('items' => $advisory_items));
    $node->field_ousa_class_advisories['und'] = array(array('value' => $advisories));
  }

  // Continue with teaching form submission fields.

  if (isset($keyed_data['class_size_limit']) && !empty($keyed_data['class_size_limit'])) {
    $node->field_ousa_class_max_size['und'] = array(array(
        'value' => $keyed_data['class_size_limit'],
      ));
  }

  if (isset($keyed_data['num_periods'][0])) {
    $node->field_ousa_class_num_periods['und'] = array(array(
        'value' => $keyed_data['num_periods'][0],
      ));
  }

  if (isset($keyed_data['materials_fee'][0]) && !empty($keyed_data['materials_fee'][0])) {
    $node->field_ousa_class_matls_fee['und'] = array(array(
        'value' => '$' . number_format($keyed_data['materials_fee'][0], 2),
      ));
  }

  if (isset($keyed_data['publication'][0]) && !empty($keyed_data['publication'][0])) {
    $node->field_ousa_class_bibliography['und'] = array(array(
        'value' => $keyed_data['publication'][0],
      ));
  }

  $node->field_ousa_class_teacher_id['und'] = array(array(
      'value' => $keyed_data['civicrm_participant_id'][0] ?? '',
    ));

  $node->field_ousa_class_sid['und'] = array(array('value' => $submission->sid));

  // These integer fields need to be initialized even though they're set later.
  $node->field_ousa_class_period['und'] = array(array('value' => 0));
  $node->field_ousa_class_rel_room['und'] = array(array('value' => 0));

  // Images

  // Remove any old images from the class node, because we'll re-load them all
  // from the webform in case they're different images with the same old name.
  $node->field_ousa_class_image['und'] = array();

  // Get the appropriate location to store the image from filefield_paths
  // settings. Note that we must have already set the year and event fields used
  // in the filefield_paths tokens in the node we are creating.
  $field_instance = field_info_instance('node', 'field_ousa_class_image', 'ousa_class');
  $tokenized_file_path = $field_instance['settings']['filefield_paths']['file_path']['value'];
  $settings = $field_instance['settings']['filefield_paths']['file_path']['options'];
  $settings['context'] = 'filename';
  $file_path = filefield_paths_process_string($tokenized_file_path,
    array('node' => $node), $settings);

  // Get the file IDs of the files that were submitted in the webform; make
  // copies of the files in the new places.
  $fids = array();
  if (isset($keyed_data['image'][0])) {
    $fids[] = $keyed_data['image'][0];
  }
  if (isset($keyed_data['image_2'][0])) {
    $fids[] = $keyed_data['image_2'][0];
  }
  if (isset($keyed_data['image_3'][0])) {
    $fids[] = $keyed_data['image_3'][0];
  }
  foreach ($fids as $fid) {
    $file = file_load($fid);
    if (empty($file)) {
      backdrop_set_message(t('Unable to load file fid = @fid.',
        array('@fid' => $fid)), 'error');
      break;
    }
    $dir = 'public://' . $file_path;
    if (!file_prepare_directory($dir, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
      backdrop_set_message(t('Unable to create or write  to directory %dir.',
        array('%dir' => $dir)), 'error');
      break;
    }
    $full_file_path = $dir . '/' . $file->filename;
    $new_file = file_copy($file, $full_file_path);
    if (!$new_file) {
      backdrop_set_message(t('Unable to create or write the file %path.',
        array('%path' => $full_file_path)), 'error');
      break;
    }
    else {
      $node->field_ousa_class_image['und'][] = array(
        'fid' => $new_file->fid,
        'display' => 1,
        'description' => '',
      );
    }
  }

  $node->field_ousa_class_approval_status['und'] = array(array('value' => 'initialized'));

  $node = node_submit($node);
  node_save($node);
  return $node;
}


// Two useful helpers

/**
 * Returns a single webform submission with the addition of keyed data.
 */
function ousa_webforms_get_submission($wid, $sid) {
  $node = node_load($wid);
  if (empty($node) || $node->type != 'webform') {
    return NULL;
  }
  module_load_include('inc', 'webform', 'includes/webform.submissions');
  $submission = webform_get_submission($wid, $sid);
  ousa_webforms_augment_submission($submission);
  return $submission;
}

/**
 * Augment a webform submission with a list of keyed data. This simplifies
 * reading (and writing) webform submission data structures.
 * @param $node - the node object.
 * @param $submission - the submission object, which will receive a new field
 * keyed_data.
 */
function ousa_webforms_augment_submission(&$submission) {
  $nid = $submission->nid;
  $form_keys = &backdrop_static(__FUNCTION__ . ':' . $nid);
  if (!isset($form_keys)) {
    $cids = ousa_webforms_cids(node_load($nid));
    $form_keys = array_flip($cids); // this maps cid to form_key
  }
  $submission->keyed_data = array();
  foreach ($submission->data as $cid => $value) {
    $submission->keyed_data[$form_keys[$cid]] = $value;
  }
}

