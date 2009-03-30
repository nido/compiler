#ifndef _RCPARSER_MESSAGE_H_
#define _RCPARSER_MESSAGE_H_

    #define RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY \
    "RCPRS-001", "Not enough memory"
    /**< The system ran out of memory */

    #define RCPARSER_ERROR_CANNOT_OPEN_FILE \
    "RCPRS-002", "Cannot open file '%s' %s"
    /**< Unable to open file for reading. */

    #define RCPARSER_ERROR_CANNOT_READ_FILE \
    "RCPRS-003", "Cannot read file '%s' %s"
    /**< Unable to open file for reading. */

    #define RCPARSER_ERROR_CANNOT_WRITE_FILE \
    "RCPRS-004", "Cannot write file '%s' %s"
    /**< Unable to open file for writing. */

    #define RCPARSER_ERROR_UNABLE_TO_GET_FILE_SIZE \
    "RCPRS-005", "Unable to get file size on file '%s'"
    /**< Unable to get file size. */

    #define RCPARSER_ERROR_FAIL_TO_PATCH_FILE \
    "RCPRS-006", "Fail to patch file"
    /**< Unable to parse original file to patch. */

    #define RCPARSER_ERROR_GETEXTINFO_NOT_SUPPORTED \
    "RCPRS-007", "getextinfo function call not supported for [%s] resource file type"
    /**< getextinfo function call not supported for this resource file type */

    #define RCPARSER_INTERNAL_ERROR_FLEXCC_NO_MORE_SUPPORTED \
    "RCPRS-008", "RCParser does not support FlexCC mode anymore"
    /**< RCParser does not support FlexCC mode anymore */
    
    #define RCPARSER_INTERNAL_ERROR_NO_COMPILER_NO_MORE_SUPPORTED \
    "RCPRS-009", "RCParser does not support reconfiguration without compiler mode anymore"
    /**< RCParser does not support reconfiguration without compiler mode anymore */
    
#endif
