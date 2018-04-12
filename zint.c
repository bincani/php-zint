#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <zint.h>

#include "php.h"
#include "ext/standard/info.h"
#include "php_version.h"

#define PHP_ZINT_VERSION "0.5"
#define PHP_ZINT_EXTNAME "zint"
#define ZINT_VERSION "2.4.1"

extern zend_module_entry zint_module_entry;
#define phpext_zint_ptr &zint_module_entry

// declaration of zint_barcode_file()
PHP_FUNCTION(zint_barcode_file);

// list of custom PHP functions provided by this extension
// set {NULL, NULL, NULL} as the last record to mark the end of list
static zend_function_entry zint_functions[] = {
    PHP_FE(zint_barcode_file, NULL)
    {NULL, NULL, NULL}
};

PHP_MINFO_FUNCTION(zint)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "zint support", "enabled");
    php_info_print_table_row(2, "version of php extension", PHP_ZINT_VERSION);
    php_info_print_table_row(2, "version of libzint", ZINT_VERSION);
    php_info_print_table_end();
}

// the following code creates an entry for the module and registers it with Zend.
zend_module_entry zint_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_ZINT_EXTNAME,
    zint_functions,
    NULL, // name of the MINIT function or NULL if not applicable
    NULL, // name of the MSHUTDOWN function or NULL if not applicable
    NULL, // name of the RINIT function or NULL if not applicable
    NULL, // name of the RSHUTDOWN function or NULL if not applicable
    PHP_MINFO(zint),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_ZINT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(zint)

// implementation of zint_barcode_file()
PHP_FUNCTION(zint_barcode_file)
{
	// structure to hold the barcode
	struct zint_symbol *barcode;

	// holds error_code after barcode generation
	int error_code = 0;

	// used for iterating over the optional_params
	zval **data;
	HashTable *params_hash;
	HashPosition pointer;
	//int params_count;
	char *key;
	int key_len;
	long index;


#if PHP_MAJOR_VERSION == 7
    zend_long type;
    zend_long rotate_angle;
    size_t input_len;
    size_t output_file_len;
#else
    long type;
    long rotate_angle;
    int input_len;
    int output_file_len;
#endif

	char *input;
	char *output_file;

	// receives the optional parameters array
	zval *optional_params;

	// parse out parameters passed
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lss|la",
	   &type, &input, &input_len, &output_file, &output_file_len, &rotate_angle, &optional_params) == FAILURE) {
		RETURN_NULL();
	}

	// only generate barcode with input data
	if (input_len > 0) {
		// initialize barcode structure
		barcode = ZBarcode_Create();

		// set barcode type
		barcode->symbology = ((int)type > 200) ? 1 : (int)type;

		// set output file
		strcpy(barcode->outfile, (output_file_len > 0) ? output_file : "barcode.png");

		// configure barcode from optional_params
#if PHP_MAJOR_VERSION == 7

    	zend_string *key;
    	zval *val;

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(optional_params), key, val) {
            if (strcmp(ZSTR_VAL(key), "height") == 0) {
    			if (Z_TYPE_P(val) == IS_LONG && Z_LVAL_P(val) < 1000) {
    				barcode->height = Z_LVAL_P(val);
    			}
    			else {
    				strcpy(barcode->errtxt, "error: given value for height is invalid");
    				error_code = 1;
    			}
			}
			else if (strcmp(ZSTR_VAL(key), "whitespace_width") == 0) {
				if (Z_TYPE_P(val) == IS_LONG && Z_LVAL_P(val) < 1000) {
					barcode->whitespace_width = Z_LVAL_P(val);
				}
				else {
					strcpy(barcode->errtxt, "error: given value for whitespace_width is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "border_width") == 0) {
				if (Z_TYPE_P(val) == IS_LONG && Z_LVAL_P(val) < 1000) {
					barcode->border_width = Z_LVAL_P(val);
				}
				else {
					strcpy(barcode->errtxt, "error: given value for border_width is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "output_options") == 0) {
				if (Z_TYPE_P(val) == IS_STRING && (strcmp(Z_STRVAL_P(val), "bind") == 0 || strcmp(Z_STRVAL_P(val), "box") == 0)) {
					if (strcmp(Z_STRVAL_P(val), "bind") == 0) {
						barcode->output_options = BARCODE_BIND;
					}
					else {
						barcode->output_options = BARCODE_BOX;
					}
				}
				else {
					strcpy(barcode->errtxt, "error: given value for output_options is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "fgcolour") == 0) {
				if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 6) {
					strcpy(barcode->fgcolour, Z_STRVAL_P(val));
				}
				else {
					strcpy(barcode->errtxt, "error: given value for fgcolour is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "bgcolour") == 0) {
			    if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 6) {
			        strcpy(barcode->bgcolour, Z_STRVAL_P(val));
				}
				else {
					strcpy(barcode->errtxt, "error: given value for bgcolour is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "scale") == 0) {
				if (Z_TYPE_P(val) == IS_DOUBLE) {
					barcode->scale = Z_DVAL_P(val);
				}
				else {
					strcpy(barcode->errtxt, "error: given value for scale is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "input_mode") == 0) {
				if (
				    Z_TYPE_P(val) == IS_STRING
				    &&
				    (
				        strcmp(Z_STRVAL_P(val), "data") == 0
				        ||
				        strcmp(Z_STRVAL_P(val), "unicode") == 0
				        ||
				        strcmp(Z_STRVAL_P(val), "gs1") == 0)
                ) {
					if (strcmp(Z_STRVAL_P(val), "unicode") == 0) {
						barcode->input_mode = UNICODE_MODE;
					}
					else if (strcmp(Z_STRVAL_P(val), "gs1") == 0) {
						barcode->input_mode = GS1_MODE;
					}
					else {
						barcode->input_mode = DATA_MODE;
					}
				}
				else {
					strcpy(barcode->errtxt, "error: given value for input_mode is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "primary") == 0) {
				if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) > 0) {
					strcpy(barcode->primary, Z_STRVAL_P(val));
				}
				else {
					strcpy(barcode->errtxt, "error: given value for primary is invalid");
					error_code = 1;
				}
			}
			else if (strcmp(ZSTR_VAL(key), "notext") == 0) {
				if (Z_TYPE_P(val) == IS_LONG && (Z_LVAL_P(val) == 0 || Z_LVAL_P(val) == 1)) {
					// show human readable text
					barcode->show_hrt = (Z_LVAL_P(val) == 1 ? 0 : 1);
				}
				else {
					strcpy(barcode->errtxt, "error: given value for notext is invalid");
					error_code = 1;
				}
			}
		} ZEND_HASH_FOREACH_END();
#else
        params_hash = Z_ARRVAL_P(optional_params);
		for (
		    zend_hash_internal_pointer_reset_ex(params_hash, &pointer);
            zend_hash_get_current_data_ex(params_hash, (void **)&data, &pointer) == SUCCESS;
            zend_hash_move_forward_ex(params_hash, &pointer)) {
			zval temp = **data;
			zval_copy_ctor(&temp);
            if (zend_hash_get_current_key_ex(params_hash, &key, &key_len, &index, 0, &pointer) == HASH_KEY_IS_STRING) {
				if (strcmp(key, "height") == 0) {
					if (Z_TYPE(temp) == IS_LONG && Z_LVAL(temp) < 1000) {
						barcode->height = Z_LVAL(temp);
					} else {
						strcpy(barcode->errtxt, "error: given value for height is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "whitespace_width") == 0) {
					if (Z_TYPE(temp) == IS_LONG && Z_LVAL(temp) < 1000) {
						barcode->whitespace_width = Z_LVAL(temp);
					} else {
						strcpy(barcode->errtxt, "error: given value for whitespace_width is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "border_width") == 0) {
					if (Z_TYPE(temp) == IS_LONG && Z_LVAL(temp) < 1000) {
						barcode->border_width = Z_LVAL(temp);
					} else {
						strcpy(barcode->errtxt, "error: given value for border_width is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "output_options") == 0) {
					if (Z_TYPE(temp) == IS_STRING && (strcmp(Z_STRVAL(temp), "bind") == 0 || strcmp(Z_STRVAL(temp), "box") == 0)) {
						if (strcmp(Z_STRVAL(temp), "bind") == 0) {
							barcode->output_options = BARCODE_BIND;
						} else {
							barcode->output_options = BARCODE_BOX;
						}
					} else {
						strcpy(barcode->errtxt, "error: given value for output_options is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "fgcolour") == 0) {
					if (Z_TYPE(temp) == IS_STRING && Z_STRLEN(temp) == 6) {
						strcpy(barcode->fgcolour, Z_STRVAL(temp));
					} else {
						strcpy(barcode->errtxt, "error: given value for fgcolour is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "bgcolour") == 0) {
					if (Z_TYPE(temp) == IS_STRING && Z_STRLEN(temp) == 6) {
						strcpy(barcode->bgcolour, Z_STRVAL(temp));
					} else {
						strcpy(barcode->errtxt, "error: given value for bgcolour is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "scale") == 0) {
					if (Z_TYPE(temp) == IS_DOUBLE) {
						barcode->scale = Z_DVAL(temp);
					} else {
						strcpy(barcode->errtxt, "error: given value for scale is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "input_mode") == 0) {
					if (Z_TYPE(temp) == IS_STRING && (strcmp(Z_STRVAL(temp), "data") == 0 || strcmp(Z_STRVAL(temp), "unicode") == 0 || strcmp(Z_STRVAL(temp), "gs1") == 0)) {
						if (strcmp(Z_STRVAL(temp), "unicode") == 0) {
							barcode->input_mode = UNICODE_MODE;
						} else if (strcmp(Z_STRVAL(temp), "gs1") == 0) {
							barcode->input_mode = GS1_MODE;
						} else {
							barcode->input_mode = DATA_MODE;
						}
					} else {
						strcpy(barcode->errtxt, "error: given value for input_mode is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "primary") == 0) {
					if (Z_TYPE(temp) == IS_STRING && Z_STRLEN(temp) > 0) {
						strcpy(barcode->primary, Z_STRVAL(temp));
					} else {
						strcpy(barcode->errtxt, "error: given value for primary is invalid");
						error_code = 1;
					}
				} else if (strcmp(key, "notext") == 0) {
					if (Z_TYPE(temp) == IS_LONG && (Z_LVAL(temp) == 0 || Z_LVAL(temp) == 1)) {
						// show human readable text
						barcode->show_hrt = Z_LVAL(temp);
					} else {
						strcpy(barcode->errtxt, "error: given value for notext is invalid");
						error_code = 1;
					}
				}
			}
			zval_dtor(&temp);
		}
#endif
		// encode barcode and save file
		if (error_code == 0) {
			error_code = ZBarcode_Encode_and_Print(barcode, input, input_len, (rotate_angle > 270) ? 0 : (int)rotate_angle);
		}
		if(error_code == 0) {
			// return true on success
			RETVAL_BOOL(1);
		} else {
			// return error_code on error
#if PHP_MAJOR_VERSION == 7
            RETVAL_STRINGL(barcode->errtxt, 1);
#else
            RETVAL_STRING(barcode->errtxt, 1);
#endif
		}
		// cleanup structure
		ZBarcode_Delete(barcode);
	}
	else {
#if PHP_MAJOR_VERSION == 7
        RETVAL_STRINGL("error: no input data given", 1);
#else
        RETVAL_STRING("error: no input data given", 1);
#endif
	}
}