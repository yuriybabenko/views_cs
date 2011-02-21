<?php

class views_handler_filter_current_source_language extends views_handler_filter_in_operator
{
	const DEFAULT_CURRENT_LANGUAGE 	= 1;
	const CURRENT_LANGUAGE			= 2;

	function query() {
		$this->ensure_my_table();

		//figure out which filtering mode we're going to use; default to both default & current language option
			$filter_mode = self::DEFAULT_CURRENT_LANGUAGE;

			if (!empty($this->view->exposed_input) && isset($this->options['expose']['identifier'])) {
				$filter_mode = $this->view->exposed_input[$this->options['expose']['identifier']];
			}

		//create the join
			$join = $this->get_join();

			if (!isset($join->extra)) {
				$join->extra = array();
			}

			$join->definition['field'] = 'nid';
			$join->field = 'nid';

			$other_alias = $this->query->add_table($this->table, $this->relationship, $join, 'other_node');

		//add the filter condition
			switch($filter_mode) {
				case 1:
					$where =	"
    									(
    										(
    											".$this->table_alias.".language = '%s'
    												OR
    											".$this->table_alias.".language = ''
    												OR
    											".$this->table_alias.".language IS NULL
    										)
    										OR
    										(
    											".$this->table_alias.".language = '%s'
    												AND
    											".$other_alias.".language IS NULL
    												AND
    											".$this->table_alias.".status = 1
    										)
    									)
    								";

					$args = array(
						i18n_get_lang(),
						i18n_default_language()
					);

					break;

				case 2:
					$where = $this->table_alias.".language = '%s'";

					$args = array(
						i18n_get_lang(),
					);

					break;
			}

			if (!empty($where)) {
				$this->query->add_where($this->options['group'], $where, $args);
			}
	}

	function get_value_options() {
		//make sure we don't overwrite existing data
	    if (!isset($this->value_options)) {
  			$this->value_title = t('Language');
  
  			global $language;
  
  			$current = $language->native;
  			$default = language_default('name');;
  
  			//show generic descriptions on views config page
  			if (arg(0) == 'admin') {
  				$default_text = t("Default site language & current user's language");
  				$current_text = t("Current user's language");
  			}
  			//show language-specific descriptions on front-end
  			else {
  				$default_text = t("@default and @current", array('@default' => $default, '@current' => $current));
  				$current_text = t("@current", array('@current' => $current));
  			}
  
  			$options = array(
  				self::DEFAULT_CURRENT_LANGUAGE	=> $default_text,
  				self::CURRENT_LANGUAGE 			=> $current_text,
  			);
  
  			$this->value_options = $options;
	    }
	}

	//overriding to hide the exposed language filter when the user's language is the same as the
	//site's default language
	function value_form(&$form, &$form_state) {
		$form['value'] = array();

    $this->get_value_options();
    $options = $this->value_options;
    $default_value = (array) $this->value;

    $which = 'all';
    if (!empty($form['operator'])) {
      $source = ($form['operator']['#type'] == 'radios') ? 'radio:options[operator]' : 'edit-options-operator';
    }

  	if (!empty($form_state['exposed'])) {
  		$identifier = $this->options['expose']['identifier'];

  		if (empty($this->options['expose']['use_operator']) || empty($this->options['expose']['operator'])) {
    		//exposed and locked.
    		$which = in_array($this->operator, $this->operator_values(1)) ? 'value' : 'none';
  		}
  		else {
      	$source = 'edit-' . form_clean_id($this->options['expose']['operator']);
    	}

    	if (!empty($this->options['expose']['reduce'])) {
        $options = $this->reduce_value_options();

        if (empty($this->options['expose']['single']) && !empty($this->options['expose']['optional'])) {
          $default_value = array();
        }
    	}

    	if (!empty($this->options['expose']['single'])) {
      	if (!empty($this->options['expose']['optional']) && (empty($default_value) || !empty($this->options['expose']['reduce']))) {
        		$default_value = 'All';
      	}
        elseif (empty($default_value)) {
        	$keys = array_keys($options);
          	$default_value = array_shift($keys);
        }
      	else {
        		$copy = $default_value;
        		$default_value = array_shift($copy);
      	}
    	}
    }

	    if ($which == 'all' || $which == 'value') {
    		$form['value'] = array(
	        '#type' => $this->value_form_type,
	        '#title' => $this->value_title,
	        '#options' => $options,
	        '#default_value' => $default_value,
	        // These are only valid for 'select' type, but do no harm to checkboxes.
	        '#multiple' => TRUE,
	        '#size' => count($options) > 8 ? 8 : count($options),
      	);

      	//hide the exposed language filter when the user's language is set to site's default language
      	if (arg(0) != 'admin' && i18n_get_lang() == i18n_default_language()) {
      		$form['value'] = array(
      			'#type' => 'hidden',
      			'#value' => self::DEFAULT_CURRENT_LANGUAGE,
      		);
      	}

      	if(!empty($form_state['exposed']) && !isset($form_state['input'][$identifier])) {
        	$form_state['input'][$identifier] = $default_value;
     	}

	    $process = array();
     	if ($this->value_form_type == 'checkboxes') {
	        // If this form element will use checkboxes in the UI, we need to
	        // check_plain() all the options ourselves since FAPI is inconsistent
	        // about this. However, instead of directly doing that to the #options
	        // right now, we define a #process callback since we might change our
	        // mind later and convert this into a 'select' form element, which
	        // would lead to double-escaping the options.
	        $process[] = 'views_process_check_options';
      	}

      	if ($which == 'all') {
	        if (empty($form_state['exposed']) && ($this->value_form_type == 'checkboxes' || $this->value_form_type == 'radios')) {
          		$process[] = "expand_$this->value_form_type";
          		$form['value']['#prefix'] = '<div id="edit-options-value-wrapper">';
          		$form['value']['#suffix'] = '</div>';
	        }

	        $process[] = 'views_process_dependency';
	        $form['value']['#dependency'] = array($source => $this->operator_values(1));
      	}

      	if (!empty($process)) {
        	$form['value']['#process'] = $process;
      	}
	    }
	}
}
