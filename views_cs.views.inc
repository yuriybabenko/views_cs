<?php

/*
 * Implementation of hook_views_data();
 */
function views_cs_views_data() {
	$data['node']['current_source_language'] = array(
		'group' => t('Node translation'),
		'title' => t('Current or source language'),
		'help'	=> t('Nodes in the current language or the source language if translation is not available.'),
		'filter' => array(
      'handler' => 'views_handler_filter_current_source_language',
		),
	);

	return $data;
}

/*
 * Implementation of hook_views_handlers();
 */
function views_cs_views_handlers() {
	return array(
		'info' => array(
			'path' => VIEWS_CS_PATH.'/includes',
		),
		'handlers' => array(
			'views_handler_filter_current_source_language' => array(
				'parent' => 'views_handler_filter_in_operator',
			),
		),
	);
}