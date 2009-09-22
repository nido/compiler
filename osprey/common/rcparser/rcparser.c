#include "utils_hostdefs.h"
#include "utils_memory.h"

#include "rcparser.h"
#include "rcparser_message.h"

typedef enum {
  NONE,
  REMOVE_BLOCK,
  ADD_BLOCK,
} ActionT;

typedef struct Block {
  char *header;
  char *header_start;
  char *header_end;

  char *begin;
  char *end;

  char *body_start;
  char *body_end;

  int  must_not_be_written;

  int  delimiter_counter;
  
  char * architecture;
  char * exthwtype;
} BlockT;

typedef struct Patch {
  char *buffer_begin;
  char *buffer_end;

  BlockT macro_block;

  BlockT block_def;

  BlockT *block_to_patch;
  int    block_to_patch_nb;

  ActionT action;

  BlockT *found_blocks;
  int    current_block_nb_used;
  int    block_nb;
  
  RCparser_rctypeT rctype;
} PatchT;

static PatchT patch;

static char *str_uc( char *str ) {
  int length = strlen(str);
  char *result = getmem_curarea(length + 1);
  int i;
  int offset = 'a' - 'A';

  if(result == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }

  for(i=0; i<length; i++) {
    result[i] = str[i];
    if('a' <= str[i] && str[i] <= 'z') {
      result[i] = str[i] - offset;
    }
  }

  result[length] = '\0';

  return result;
}

static char *
delete_comments( char *str ) {
  int length = strlen(str);
  int start_comment = FALSE;
  int i;
  int idx;

  for(i=0, idx = 0;i<length;i++) {
    if(str[i] == '#') { start_comment = TRUE; }

    if(start_comment && (str[i] == '\n' || str[i] == '\r')) {
      start_comment = FALSE;
    }

    if( ! start_comment ) { str[idx++] = str[i]; }
  }

  str[idx] = '\0';
  
  return str;
}

static char *
get_body_without_starting_space_and_cr( char *block_body ) {

  while( *block_body != '\0' && (*block_body == ' ' || *block_body == '\t') ) { block_body++; }
  if( *block_body != '\0' && (*block_body == '\n' ||
			      *block_body == '\r') ) { block_body++; }
  return block_body;
}

static void
check_found_blocks_size ( void ) {

  if(patch.current_block_nb_used == patch.block_nb) {
    BlockT *new_blocks;
    int i;

    /* must reallocate array. */
    patch.block_nb *= 2;

    new_blocks = (BlockT*) getmem_curarea(sizeof(BlockT) * patch.block_nb);
    if(new_blocks == NULL) {
      utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
    }
    
    new_blocks = memset(new_blocks, 0, sizeof(BlockT) * patch.block_nb);

    for(i=0; i <= patch.current_block_nb_used; i++) {
      new_blocks[i] = patch.found_blocks[i];
    }

    patch.found_blocks = new_blocks;
  }

}

static char * get_architecture( char * buf ) {
  char * architecture;

  architecture = strstr(buf,"ARCHITECTURE=");
  if (NULL!=architecture) {
    char * back_end;
    
    architecture += 13;
    back_end = strchr(architecture,'\n');
    if (back_end!=NULL) *back_end = 0;
    architecture = strdup(architecture);
    if (back_end!=NULL) *back_end = '\n';
  } else {
    architecture = strdup("stxp70v3");
  }
  return architecture;
}

static char * get_exthwtype( char * buf ) {
  char * exthwtype;

  exthwtype = strstr(buf,"EXTHWTYPE=");
  if (NULL!=exthwtype) {
    char * back_end;
    
    exthwtype += 10;
    back_end = strchr(exthwtype,'\n');
    if (back_end!=NULL) *back_end = 0;
    exthwtype = strdup(exthwtype);
    if (back_end!=NULL) *back_end = '\n';
  } else {
    exthwtype = strdup("novliw");
  }
  return exthwtype;
}

static void
add_block( char *block_header, char *block_body ) {
  check_found_blocks_size();

  patch.found_blocks[patch.current_block_nb_used].header_start = block_header;
  patch.found_blocks[patch.current_block_nb_used].architecture = get_architecture(block_body);
  patch.found_blocks[patch.current_block_nb_used].exthwtype = get_exthwtype(block_body);
  patch.found_blocks[patch.current_block_nb_used++].body_start = block_body;
}

static void
insert_block( int insert_idx, char *block_header, char *block_body ) {
  int i;

  check_found_blocks_size();
  patch.current_block_nb_used++;

  for(i= patch.current_block_nb_used - 1; i > insert_idx; i--) {
    patch.found_blocks[i] = patch.found_blocks[i-1];
  }

  patch.found_blocks[insert_idx].header_start = block_header;
  patch.found_blocks[insert_idx].architecture = get_architecture(block_body);
  patch.found_blocks[insert_idx].exthwtype = get_exthwtype(block_body);
  patch.found_blocks[insert_idx].body_start = block_body;
}

static 
int getFileSize( char *filename, char * ModuleName ) {
  struct stat file_infos;

  ASSERT(filename != NULL, ModuleName, "Unable to get file size because name not given");

  if(stat(filename,&file_infos) == -1) {
    utilsPrintFatal(RCPARSER_ERROR_CANNOT_OPEN_FILE, filename, utilsPrintSystemError());
    return -1;
  }

  return file_infos.st_size *sizeof(char) + 1;
}

static int
find_common_char( char *input_buffer, char *ref_buffer, int skip_space, int skip_tab) {

  int nb_char = 0;

  if(*input_buffer == *ref_buffer) { 
    return 1;
  }

  if(skip_space && *input_buffer == ' ') {
    return -1;
  }

  if(skip_tab && *input_buffer == '\t') {
    return -1;
  }

  if(skip_space && skip_tab) {
    while(*ref_buffer == ' ' || *ref_buffer == '\t') {
      ref_buffer++;
      nb_char++;
    }
    if(*input_buffer == *ref_buffer) {
      return nb_char + 1;
    }
  }
  else if(skip_space) {
    while(*ref_buffer == ' ') {
      ref_buffer++;
      nb_char++;
    }
    if(*input_buffer == *ref_buffer) {
      return nb_char + 1;
    }
  }
  else if(skip_tab) {
    while(*ref_buffer == '\t') {
      ref_buffer++;
      nb_char++;
    }
    if(*input_buffer == *ref_buffer) {
      return nb_char + 1;
    }
  }

  return 0;
}

static char *
find_macro_block_begin( char *buffer ) {

  char *buffer_ptr = buffer;
  char *macro_block_begin_ptr = patch.macro_block.begin;

  while(*buffer_ptr != '\0') {
    int nb_char = find_common_char(buffer_ptr,macro_block_begin_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      macro_block_begin_ptr += nb_char;
      if(*(macro_block_begin_ptr) == '\0') {
	return ++buffer_ptr;
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      macro_block_begin_ptr = patch.macro_block.begin;
    }

    buffer_ptr++;
  }

  return (char*)NULL;
}

static char *
find_macro_block_end( char *buffer ) {

  char *buffer_ptr = buffer;
  char *macro_block_end_ptr = patch.macro_block.end;

  while(*buffer_ptr != '\0') {
    int nb_char = find_common_char(buffer_ptr,macro_block_end_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      macro_block_end_ptr += nb_char;
      if(*(macro_block_end_ptr) == '\0') {
	return ++buffer_ptr;
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      macro_block_end_ptr = patch.macro_block.end;
    }

    buffer_ptr++;
  }

  return (char*)NULL;
}

static int
get_macro_block_size( char *buffer ) {

  char *buffer_ptr = buffer;
  char *buffer_ptr_before_last_end_block_delimiter = NULL;
  int find_escape = FALSE;
  int had_escape = FALSE;

  char *macro_block_begin_ptr = patch.macro_block.begin;
  char *macro_block_end_ptr = patch.macro_block.end;

  while(*buffer_ptr != '\0') {
    int nb_char;

    if(find_escape) {
      find_escape = FALSE;
      had_escape = TRUE;
    }

    if((macro_block_begin_ptr == patch.macro_block.begin ||
       macro_block_end_ptr == patch.macro_block.end) &&
       *buffer_ptr == '\\') {

      find_escape = TRUE; 
    }

    nb_char = find_common_char(buffer_ptr,macro_block_begin_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      if(! (macro_block_begin_ptr == patch.macro_block.begin && had_escape) ) { 
	macro_block_begin_ptr += nb_char;
	if(*(macro_block_begin_ptr) == '\0') {
	  patch.macro_block.delimiter_counter++;
	  UTILS_DEBUG( "GRC-PTCH", ( "macro delimiters number is %d\n",patch.macro_block.delimiter_counter));
	  macro_block_begin_ptr = patch.macro_block.begin;
	}
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      macro_block_begin_ptr = patch.macro_block.begin;
    }

    nb_char = find_common_char(buffer_ptr,macro_block_end_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      if(! (macro_block_end_ptr == patch.macro_block.end && had_escape) ) { 
	if(macro_block_end_ptr == patch.macro_block.end) {
	  buffer_ptr_before_last_end_block_delimiter = buffer_ptr;
	}
	macro_block_end_ptr += nb_char;
	if(*(macro_block_end_ptr) == '\0') {
	  macro_block_end_ptr = patch.macro_block.end;
	  patch.macro_block.delimiter_counter--;
	  UTILS_DEBUG( "GRC-PTCH", ( "macro delimiters number is %d\n",patch.macro_block.delimiter_counter));
	  if(patch.macro_block.delimiter_counter == 0) {
	    UTILS_DEBUG( "GRC-PTCH", ( "End delimiter is at:%s\n",buffer_ptr+1));
	    return buffer_ptr_before_last_end_block_delimiter - buffer;
	  }
	}
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      macro_block_end_ptr = patch.macro_block.end;
    }

    had_escape = FALSE;

    buffer_ptr++;
  }

  return -1;
}

static char *
find_macro_block_header( char *buffer ) {

  char *buffer_ptr = buffer;
  char *macro_block_header_ptr = patch.macro_block.header;

  if(*patch.macro_block.header == '\0') { return buffer; }

  while(*buffer_ptr != '\0') {
    /* Omit comment lines */
    if (*buffer_ptr == '#') {
      char * next_newline;
      
      next_newline = strchr(buffer_ptr,'\n');
      if (NULL!=next_newline) {
      	buffer_ptr=next_newline;
      } else {
        patch.macro_block.header_end = NULL;
        return (char*)NULL;
      }
    } else {
      int nb_char = find_common_char(buffer_ptr,macro_block_header_ptr, TRUE, TRUE);
      if(nb_char != 0 && nb_char != -1 ) {
        if(patch.macro_block.header == macro_block_header_ptr) {
	  patch.macro_block.header_start = buffer_ptr;
        }
        macro_block_header_ptr += nb_char;
        if(*macro_block_header_ptr == '\0') {
	  patch.macro_block.header_end = ++buffer_ptr;
  	  return buffer_ptr;
        }
      }
      else if(nb_char == -1) {
        /* let increment the buffer_ptr */
      }
      else {
        macro_block_header_ptr = patch.macro_block.header;
      }
    }
    buffer_ptr++;
  }

  patch.macro_block.header_end = NULL;
  return (char*)NULL;
}

static int
find_macro_block_body( char *buffer ) {

  char *buffer_ptr = find_macro_block_begin(buffer);
  int macro_block_size;

  patch.macro_block.body_start = buffer_ptr;

  if(buffer_ptr == NULL) { return FALSE; }

  patch.macro_block.delimiter_counter++;

  macro_block_size = get_macro_block_size( buffer_ptr );
  if(macro_block_size == -1) { return FALSE; }

  patch.macro_block.body_end = buffer_ptr + macro_block_size;

  return TRUE;
}

static char *
find_block_begin( char *buffer ) {

  char *buffer_ptr = buffer;
  char *block_begin_ptr = patch.block_def.begin;

  while(*buffer_ptr != '\0') {
    int nb_char = find_common_char(buffer_ptr,block_begin_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      block_begin_ptr += nb_char;
      if(*(block_begin_ptr) == '\0') {
	return ++buffer_ptr;
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      block_begin_ptr = patch.block_def.begin;
    }

    buffer_ptr++;
  }

  return (char*)NULL;
}

static char *
get_block_header( char *from, char *to ) {
  int block_begin_is_found = FALSE;
  int found_end_of_header = FALSE;
  char *buffer_ptr = to;
  int header_size = 0;
  char *header;

  while(buffer_ptr >= from) {
    if(block_begin_is_found) {
      if( ! found_end_of_header) {
	if( ! (*buffer_ptr == ' '  ||
	       *buffer_ptr == '\t' ||
	       *buffer_ptr == '\n' ||
	       *buffer_ptr == '\r'
	       ) ) {
	  found_end_of_header = TRUE;
	}
      }

      if(found_end_of_header) {
	if(*buffer_ptr == ' ' || *buffer_ptr == '\t' || buffer_ptr == from) {
	  int i;
	  header = (char*) getmem_curarea(header_size + 1);
	  if(header == NULL) {
	    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
	  }
	  if(*buffer_ptr == ' ' || *buffer_ptr == '\t') { buffer_ptr++; }
	  for(i = 0; i < header_size; i++) {
	    header[i] = buffer_ptr[i];
	  }
	  header[header_size] = '\0';
	  return header;
	}
	else {
	  header_size++;
	}
      }
    }
    else {
      char *tmp_str;
      if((tmp_str = find_block_begin(buffer_ptr)) != NULL) {
	if(tmp_str == buffer_ptr + strlen(patch.block_def.begin)) {
	  block_begin_is_found = TRUE;
	}
      }
    }

    buffer_ptr--;
  }

  return (char*)NULL;
}

static char *
find_block_end( char *buffer ) {

  char *buffer_ptr = buffer;
  char *block_end_ptr = patch.block_def.end;

  while(*buffer_ptr != '\0') {
    int nb_char = find_common_char(buffer_ptr,block_end_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      block_end_ptr += nb_char;
      if(*(block_end_ptr) == '\0') {
	return ++buffer_ptr;
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      block_end_ptr = patch.block_def.end;
    }

    buffer_ptr++;
  }

  return (char*)NULL;
}

static int
get_block_size( char *buffer ) {

  char *buffer_ptr = buffer;
  char *buffer_ptr_before_last_end_block_delimiter = NULL;
  int find_escape = FALSE;
  int had_escape = FALSE;

  char *block_begin_ptr = patch.block_def.begin;
  char *block_end_ptr = patch.block_def.end;

  while(*buffer_ptr != '\0') {
    int nb_char;

    if(find_escape) {
      find_escape = FALSE;
      had_escape = TRUE;
    }

    if((block_begin_ptr == patch.block_def.begin ||
       block_end_ptr == patch.block_def.end) &&
       *buffer_ptr == '\\') {

      find_escape = TRUE; 
    }

    nb_char = find_common_char(buffer_ptr,block_begin_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      if(! (block_begin_ptr == patch.block_def.begin && had_escape) ) { 
	block_begin_ptr += nb_char;
	if(*(block_begin_ptr) == '\0') {
	  patch.block_def.delimiter_counter++;
	  UTILS_DEBUG( "GRC-PTCH", ( "delimiters number is %d\n",patch.block_def.delimiter_counter));
	  block_begin_ptr = patch.block_def.begin;
	}
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      block_begin_ptr = patch.block_def.begin;
    }

    nb_char = find_common_char(buffer_ptr,block_end_ptr, TRUE, TRUE);
    if(nb_char != 0 && nb_char != -1 ) {
      if(! (block_end_ptr == patch.block_def.end && had_escape) ) { 
	if(block_end_ptr == patch.block_def.end) {
	  buffer_ptr_before_last_end_block_delimiter = buffer_ptr;
	}
	block_end_ptr += nb_char;
	if(*(block_end_ptr) == '\0') {
	  block_end_ptr = patch.block_def.end;
	  patch.block_def.delimiter_counter--;
	  UTILS_DEBUG( "GRC-PTCH", ( "delimiters number is %d\n",patch.block_def.delimiter_counter));
	  if(patch.block_def.delimiter_counter == 0) {
	    UTILS_DEBUG( "GRC-PTCH", ( "End delimiter is at:%s\n",buffer_ptr+1));
	    return buffer_ptr_before_last_end_block_delimiter - buffer;
	  }
	}
      }
    }
    else if(nb_char == -1) {
      /* let increment the buffer_ptr */
    }
    else {
      block_end_ptr = patch.block_def.end;
    }

    had_escape = FALSE;

    buffer_ptr++;
  }

  return -1;
}

static int
find_blocks( char *from , char *to ) {
  char *block_body;
  char *block_body_end;
  char *block_header;
  int  body_size;
  int  more_blocks = TRUE;


  while( *from != '\0' && from <= to && more_blocks ) {
    
    block_body = find_block_begin( from );

    if(block_body) {
      block_header = get_block_header(from, block_body);
      UTILS_DEBUG( "GRC-PTCH", ( "found block header: '%s'\n",block_header));
      if(block_header == NULL) { return FALSE; }

      patch.block_def.delimiter_counter = 1;
      
      body_size = get_block_size( block_body );
      UTILS_DEBUG( "GRC-PTCH", ( "block size is %d\n",body_size));

      block_body_end = block_body + body_size;
      
      from = find_block_end( block_body_end );
      
      *block_body_end = '\0';

      UTILS_DEBUG( "GRC-PTCH", ( "find block at:\n%s\n", block_body));

      block_body = get_body_without_starting_space_and_cr(block_body);

      add_block(block_header,block_body);
      
      UTILS_DEBUG( "GRC-PTCH", ( "next is at:\n%s\n",from));
    }
    else {
      more_blocks = FALSE;
    }
  }

  return TRUE;
}

static void
merge_sort( BlockT *blocks, int begin, int end ) {
  unsigned int pivot;
  unsigned int length;
  BlockT *working_array;
  unsigned int i;
  unsigned int m1, m2;
  
  if(begin == end) { return; }
  length = end-begin+1;
  pivot = (begin + end) / 2;
  merge_sort(blocks, begin, pivot);
  merge_sort(blocks, pivot+1,end);

  working_array = (BlockT*) getmem_curarea(sizeof(BlockT) * length);
  if(working_array == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }

  for(i=0; i < length; i++) {
    working_array[i] = blocks[begin+i];
  }

#if 0
  for(i=0; i < length; i++) {
    fprintf(stdout,"during merge, working: %s ( %s )\n",
	    UTILS_STRING_SAFE(working_array[i].header_start),
	    UTILS_STRING_SAFE(working_array[i].body_start) );
  }
#endif  

  m1 = 0;
  m2 = pivot - begin + 1;

  for(i=0; i < length; i++) {
    if(m2 <= (unsigned int)end-begin) {
      if(m1 <= pivot-begin) {
	if(strcmp(working_array[m1].header_start,working_array[m2].header_start) > 0) {
	  blocks[i+begin] = working_array[m2++];
	}
	else { blocks[i+begin] = working_array[m1++]; }
      }
      else { blocks[i+begin] = working_array[m2++]; }
    }
    else { blocks[i+begin] = working_array[m1++]; }
  }
}

static int
getExtensionSwitch( FILE *infile, int infile_size ) {

  int  buffer_size = infile_size;
  int  char_read;
  char *buffer;
  char *buffer_ptr;
  char *macro_block;
  char tmp_char;
  int i;

  buffer = (char*) getmem_curarea(buffer_size);
  if(buffer == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  char_read = fread(buffer, 1, buffer_size, infile);
 
  buffer[char_read] = '\0';

  patch.buffer_begin = buffer;

  buffer_ptr = buffer;

  macro_block = buffer_ptr;

  buffer_ptr = find_macro_block_header( buffer_ptr );
  if(buffer_ptr == NULL) { return FALSE; }

  *(patch.macro_block.header_start) = '\0';

  tmp_char = *(patch.macro_block.header_end);
  *patch.macro_block.header_end = '\0';
  UTILS_DEBUG( "GRC-PTCH", ("find macro block header:\n%s\n", patch.macro_block.header_start));
  *patch.macro_block.header_end = tmp_char;

  if( find_macro_block_body(buffer_ptr) == FALSE) { return FALSE; }

  tmp_char = *(patch.macro_block.body_end);
  *patch.macro_block.body_end = '\0';
  UTILS_DEBUG( "GRC-PTCH", ("find macro block body:\n%s\n", patch.macro_block.body_start));
  *patch.macro_block.body_end = tmp_char;

  patch.macro_block.body_start = delete_comments(patch.macro_block.body_start);

  if( find_blocks( patch.macro_block.body_start, patch.macro_block.body_end) == FALSE) {
    return FALSE;
  }

  patch.buffer_end = find_macro_block_end(patch.macro_block.body_end);
  if(patch.buffer_end == NULL) { return FALSE; }
  
  if (patch.current_block_nb_used!=0) {
    merge_sort(patch.found_blocks,0,patch.current_block_nb_used-1);
  }

  UTILS_DEBUG( "GRC-PTCH", ( "begining of the buffer:\n%s\n",patch.buffer_begin));
  UTILS_DEBUG( "GRC-PTCH", ( "macro block header:\n%s %s\n",patch.macro_block.header,patch.macro_block.begin));

  for(i=0; i < patch.current_block_nb_used; i++) {
    UTILS_DEBUG( "GRC-PTCH", ( "Find block '%s'-%s-%s %s\n%s%s\n",
	    patch.found_blocks[i].header_start,patch.found_blocks[i].architecture,
	    patch.found_blocks[i].exthwtype,patch.block_def.begin,
	    patch.found_blocks[i].body_start,patch.block_def.end));
  }

  UTILS_DEBUG( "GRC-PTCH", ( "after macro block header:\n %s%s\n",patch.macro_block.end,patch.buffer_end));

  return TRUE;
}

static int
applyPatch( FILE *outfile ) {
  int i, j;

  fprintf(outfile,"%s",patch.buffer_begin);
  fprintf(outfile,"%s %s\n",patch.macro_block.header,patch.macro_block.begin);

  if(patch.action == REMOVE_BLOCK) {
    for(j=0; j < patch.block_to_patch_nb; j++) {
      for(i=0; i < patch.current_block_nb_used; i++) {
	if((strcmp(patch.block_to_patch[j].header_start,patch.found_blocks[i].header_start) == 0) &&
	   (strcmp(patch.block_to_patch[j].architecture,patch.found_blocks[i].architecture) == 0) &&
	   (strcmp(patch.block_to_patch[j].exthwtype,patch.found_blocks[i].exthwtype) == 0)) {
	  patch.found_blocks[i].must_not_be_written = TRUE;
	}
      }
    }
  }
  else if(patch.action == ADD_BLOCK) {
    int find_blocks = 0;
    for(j=0; j < patch.block_to_patch_nb; j++) {
      for(i=0; i < patch.current_block_nb_used; i++) {
	if((strcmp(patch.block_to_patch[j].header_start,patch.found_blocks[i].header_start) == 0) &&
	   (strcmp(patch.block_to_patch[j].architecture,patch.found_blocks[i].architecture) == 0) &&
	   (strcmp(patch.block_to_patch[j].exthwtype,patch.found_blocks[i].exthwtype) == 0)) {
	  find_blocks++;
	  patch.found_blocks[i].body_start = patch.block_to_patch[j].body_start;
	}
      }
    }

    if(find_blocks > 0 && find_blocks != patch.block_to_patch_nb) { return FALSE; }

    if(find_blocks == 0) {
      merge_sort(patch.block_to_patch,0,patch.block_to_patch_nb-1);

      for(j=0; j < patch.block_to_patch_nb; j++) {
	int found_insert_point = FALSE;
	for(i=0; i < patch.current_block_nb_used; i++) {
	  if(strcmp(patch.block_to_patch[j].header_start,patch.found_blocks[i].header_start) <= 0) {
	    int insert_idx = i > 0 ? i : 0;
	    found_insert_point = TRUE;
	    UTILS_DEBUG( "GRC-PTCH", ( "%s is lower than %s\n",
		    patch.block_to_patch[j].header_start,patch.found_blocks[i].header_start));
	    UTILS_DEBUG( "GRC-PTCH", ( "insert index is %d\n",insert_idx));
	    insert_block(insert_idx, patch.block_to_patch[j].header_start,patch.block_to_patch[j].body_start);
	    break;
	  }
	}
	if( ! found_insert_point) {
	  UTILS_DEBUG( "GRC-PTCH", ( "insert index is %d\n",patch.current_block_nb_used));
	  found_insert_point = TRUE;
	  insert_block(patch.current_block_nb_used, patch.block_to_patch[j].header_start,patch.block_to_patch[j].body_start);
	}
      }
    }
  }

  for(i=0; i < patch.current_block_nb_used; i++) {
    if(! patch.found_blocks[i].must_not_be_written) {
      fprintf(outfile,"           %s %s\n%s%s\n",
	      patch.found_blocks[i].header_start,patch.block_def.begin,patch.found_blocks[i].body_start,
              patch.block_def.end);
    }
  }

  fprintf(outfile,"        %s%s",patch.macro_block.end,patch.buffer_end);

  return TRUE;
}

#define GET_EXTINFO(buf,str,var)                                \
{                                                               \
   char * start, * end;                                         \
   start = strstr(buf,str);                                     \
   if (NULL!=start) {                                           \
      end = strchr(start,'\n');                                 \
   } else {                                                     \
      end = NULL;                                               \
   }                                                            \
   if ((NULL!=start) && (NULL!=end)) {                          \
      var = (char *)malloc(end-start-strlen(str)+2);            \
      memset(var,0,end-start-strlen(str)+2);                    \
      strncpy(var,start+strlen(str)+1,end-start-strlen(str)-1); \
   } else {                                                     \
      var = strdup("");                                         \
   }                                                            \
}
static void Getextinfo_open64 ( char * buffer, RCparser_ExtensionInfoT * extinfo ) {
   GET_EXTINFO(buffer,"ARCHITECTURE",extinfo->Architecture)
   if (extinfo->Architecture[0]==0) extinfo->Architecture=strdup("stxp70v3");
   GET_EXTINFO(buffer,"EXTHWTYPE",extinfo->ExtHwType)
   if (extinfo->ExtHwType[0]==0) extinfo->ExtHwType=strdup("novliw");
   GET_EXTINFO(buffer,"O64LIBPATH",extinfo->CmpLibPath)
   GET_EXTINFO(buffer,"O64LIBNAME",extinfo->CmpLibName)
   GET_EXTINFO(buffer,"ASMLIBPATH",extinfo->AsmLibPath)
   GET_EXTINFO(buffer,"ASMLIBNAME",extinfo->AsmLibName)
   GET_EXTINFO(buffer,"LDLIBPATH",extinfo->LdLibPath)
   GET_EXTINFO(buffer,"LDLIBNAME",extinfo->LdLibName)
   GET_EXTINFO(buffer,"DISASMLIBPATH",extinfo->DisasmLibPath)
   GET_EXTINFO(buffer,"DISASMLIBNAME",extinfo->DisasmLibName)
   GET_EXTINFO(buffer,"EXTLIBPATH",extinfo->ToolsetLibPath)
   GET_EXTINFO(buffer,"EXTLIBNAME",extinfo->ToolsetLibName)
   GET_EXTINFO(buffer,"O64TARG",extinfo->CmpTargOpt)
   GET_EXTINFO(buffer,"DEFINES",extinfo->Defines)
   GET_EXTINFO(buffer,"INCLUDES",extinfo->Includes)
   GET_EXTINFO(buffer,"XLIBS",extinfo->XLibs)
   GET_EXTINFO(buffer,"HELP",extinfo->HelpMsg)
}
#undef GET_EXTINFO

void RCparser_Init ( RCparser_rctypeT rctype ) {

#define FLEXCC_MACROBLOCK_HEADER "switch -Mextension is\n"              \
                                 "  help(Activate STxP70 extensions)\n" \
                                 "        keyword"
#define OPEN64_MACROBLOCK_HEADER "UserDefinedExtensions"

  if (rctype == RCPARSER_OPEN64) {
     patch.macro_block.header = OPEN64_MACROBLOCK_HEADER;
  } else {
     patch.macro_block.header = FLEXCC_MACROBLOCK_HEADER;
  }
  patch.macro_block.header_start = NULL;
  patch.macro_block.header_end = NULL;

  patch.macro_block.begin = "(";
  patch.macro_block.end = ")";

  patch.macro_block.body_start = NULL;
  patch.macro_block.body_end = NULL;

  patch.macro_block.delimiter_counter = 0;


  patch.block_def.header = "\\w";
  patch.block_def.header_start = NULL;
  patch.block_def.header_end = NULL;

  patch.block_def.begin = "(";
  patch.block_def.end = ")";

  patch.block_def.body_start = NULL;
  patch.block_def.body_end = NULL;

  patch.block_def.delimiter_counter = 0;

  patch.block_to_patch = (BlockT*) getmem_curarea(sizeof(BlockT) * 2);
  if(patch.block_to_patch == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }

  patch.block_to_patch = memset(patch.block_to_patch, 0, sizeof(BlockT) * 2);

  patch.block_to_patch[0].begin  = "(";
  patch.block_to_patch[0].end    = ")";

  patch.block_nb = 16;
  patch.current_block_nb_used = 0;
  patch.found_blocks = (BlockT*) getmem_curarea(sizeof(BlockT) * patch.block_nb);
  if(patch.found_blocks == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }

  patch.found_blocks = memset(patch.found_blocks, 0, sizeof(BlockT) * patch.block_nb);

  patch.rctype = rctype;
}

void RCparser_CreatePatch ( char *lib_path, char *inc_path, char *xlib_path, char *ext_name, 
                             char *ext_long_name, int remove, int Multiplier, int CompilerDll,
			     char *ext_arch, char * ext_hwtype ) {
  static char *body_with_default_port_format_open64_stxp70v3 = 
    "              ARCHITECTURE=%s\n"
    "              EXTHWTYPE=%s\n"
    "              O64LIBPATH=%s\n"
    "              O64LIBNAME=%sopen64\n"
    "              ASMLIBPATH=%s\n"
    "              ASMLIBNAME=%sasm\n"
    "              LDLIBPATH=%s\n"
    "              LDLIBNAME=%sld\n"
    "              DISASMLIBPATH=%s\n"
    "              DISASMLIBNAME=%sdisasm\n"
    "              EXTLIBPATH=%s\n"
    "              EXTLIBNAME=%sext\n"
    "              DEFINES=-D__%s -D__%s__ -D__%s -D__%s__\n"
    "              INCLUDES=%s\n"
    "              XLIBS=%s\n"
    "              HELP=Activate %s - STxP70 v3 architecture\n"
    "           ";
  static char *body_with_default_port_format_open64_multiplier_stxp70v3 = 
    "              ARCHITECTURE=%s\n"
    "              EXTHWTYPE=%s\n"
    "              O64LIBPATH=%s\n"
    "              O64LIBNAME=%sopen64\n"
    "              ASMLIBPATH=%s\n"
    "              ASMLIBNAME=%sasm\n"
    "              LDLIBPATH=%s\n"
    "              LDLIBNAME=%sld\n"
    "              DISASMLIBPATH=%s\n"
    "              DISASMLIBNAME=%sdisasm\n"
    "              EXTLIBPATH=%s\n"
    "              EXTLIBNAME=%sext\n"
    "              DEFINES=-D__%s -D__%s__ -D__%s -D__%s__\n"
    "              INCLUDES=%s\n"
    "              XLIBS=%s\n"
    "              HELP=Activate %s - STxP70 v3 architecture\n"
    "              O64TARG=-TARG:enable_mx=on\n"
    "           ";
  static char *body_with_default_port_format_open64_stxp70v4 = 
    "              ARCHITECTURE=%s\n"
    "              EXTHWTYPE=%s\n"
    "              O64LIBPATH=%s\n"
    "              O64LIBNAME=%sopen64\n"
    "              EXTLIBPATH=%s\n"
    "              EXTLIBNAME=%sext\n"
    "              DEFINES=-D__%s -D__%s__ -D__%s -D__%s__\n"
    "              INCLUDES=%s\n"
    "              XLIBS=%s\n"
    "              HELP=Activate %s - STxP70 v4 architecture - %s extension\n"
    "           ";
  static char *body_with_default_port_format_open64_multiplier_stxp70v4 = 
    "              ARCHITECTURE=%s\n"
    "              EXTHWTYPE=%s\n"
    "              O64LIBPATH=%s\n"
    "              O64LIBNAME=%sopen64\n"
    "              EXTLIBPATH=%s\n"
    "              EXTLIBNAME=%sext\n"
    "              DEFINES=-D__%s -D__%s__ -D__%s -D__%s__\n"
    "              INCLUDES=%s\n"
    "              XLIBS=%s\n"
    "              HELP=Activate %s - STxP70 v4 architecture - %s extension\n"
    "              O64TARG=-TARG:enable_mx=on\n"
    "           ";
  char *ext_architecture=NULL;
  char *ext_exthwtype=NULL;
  char *body_with_default_port_format = NULL;
  char *inc_path1;
  char *xlib_path1;
  int   body_size;
  int   Architecture;
  char *exthwtype_helpstr = "???";
  
  if (patch.rctype != RCPARSER_OPEN64) {
  	utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_FLEXCC_NO_MORE_SUPPORTED);
  }
  if (CompilerDll != 1) {
  	utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NO_COMPILER_NO_MORE_SUPPORTED);
  }

  body_size=(strlen(inc_path)*2+strlen(ext_name)+8);
  inc_path1=(char*) getmem_curarea(body_size);
  if (inc_path1 == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }
  if (strlen(inc_path)) {
    snprintf(inc_path1,body_size,"-I%s -I%s/%s",inc_path,inc_path,ext_name);
  } else {
    inc_path1[0]=0;
  }

  body_size=(strlen(xlib_path)*2+strlen(ext_name)+8);
  xlib_path1=(char*) getmem_curarea(body_size);
  if (xlib_path1 == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }
  if (strlen(xlib_path)) {
    snprintf(xlib_path1,body_size,"-L%s/%s",xlib_path,ext_name);
  } else {
    xlib_path1[0]=0;
  }

  if ((ext_arch==NULL) || (strcmp(ext_arch,"stxp70v3")==0)) {
     ext_architecture = "stxp70v3";
     ext_exthwtype = "novliw";
     if (Multiplier) {
       body_with_default_port_format = body_with_default_port_format_open64_multiplier_stxp70v3;
     } else {
       body_with_default_port_format = body_with_default_port_format_open64_stxp70v3;
     }
     body_size = strlen(body_with_default_port_format) + /* format length */
                 strlen(ext_architecture) +              /* extension architecture */
                 strlen(ext_exthwtype) +                 /* extension HW type */
                 strlen(ext_long_name) +                 /* long name size used one time */
                 9 * strlen(ext_name) +                  /* ext name size used 9 times */
                 5 * strlen(lib_path) +                  /* shared object path used 5 times */
                 strlen(inc_path1) +                     /* include path */
                 strlen(xlib_path1) +                    /* extension libraries include path */
                 1;                                      /* ending null char */
     Architecture = 3;
  } else {
     ext_architecture = ext_arch;
     ext_exthwtype = (NULL==ext_hwtype)?"novliw":ext_hwtype;
     if (!strcmp(ext_exthwtype,"novliw")) {
       exthwtype_helpstr = "single issue";
     } else if (!strcmp(ext_exthwtype,"single")) {
       exthwtype_helpstr = "dual issue single pipeline";
     } else if (!strcmp(ext_exthwtype,"dual")) {
       exthwtype_helpstr = "dual issue dual pipeline";
     } else {
       exthwtype_helpstr = "???";
     }
     if (Multiplier) {
       body_with_default_port_format = body_with_default_port_format_open64_multiplier_stxp70v4;
     } else {
       body_with_default_port_format = body_with_default_port_format_open64_stxp70v4;
     }
     body_size = strlen(body_with_default_port_format) + /* format length */
                 strlen(ext_architecture) +              /* extension architecture */
                 strlen(ext_exthwtype) +                 /* extension HW type */
                 strlen(ext_long_name) +                 /* long name size used one time */
                 6 * strlen(ext_name) +                  /* ext name size used 6 times */
                 2 * strlen(lib_path) +                  /* shared object path used 2 times */
                 strlen(inc_path1) +                     /* include path */
                 strlen(xlib_path1) +                    /* extension libraries path */
                 strlen(exthwtype_helpstr) +             /* extension HW type */
                 1;                                      /* ending null char */
     Architecture = 4;
  }

  patch.block_to_patch[0].header_start = (char*) getmem_curarea(strlen(ext_name) + 1);
  if(patch.block_to_patch[0].header_start == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }

  strcpy(patch.block_to_patch[0].header_start,ext_name);
  patch.block_to_patch[0].architecture = strdup(ext_architecture);
  patch.block_to_patch[0].exthwtype = strdup(ext_exthwtype);

  patch.block_to_patch[0].body_start = getmem_curarea(body_size);
  if (patch.block_to_patch[0].body_start == NULL) {
    utilsPrintInternalError(RCPARSER_INTERNAL_ERROR_NOT_ENOUGH_MEMORY);
  }
  if (Architecture==3) {
    sprintf(patch.block_to_patch[0].body_start,
            body_with_default_port_format,
	    ext_architecture,
	    ext_exthwtype,
	    lib_path,ext_name,
  	    lib_path,ext_name,
	    lib_path,ext_name,
	    lib_path,ext_name,
  	    lib_path,ext_name,
	    str_uc(ext_name),
	    str_uc(ext_name),
	    ext_name,
	    ext_name,
	    inc_path1,
	    xlib_path1,
  	    ext_long_name);
  } else {
    sprintf(patch.block_to_patch[0].body_start,
            body_with_default_port_format,
	    ext_architecture,
	    ext_exthwtype,
	    lib_path,ext_name,
  	    lib_path,ext_name,
	    str_uc(ext_name),
	    str_uc(ext_name),
	    ext_name,
	    ext_name,
	    inc_path1,
	    xlib_path1,
  	    ext_long_name,exthwtype_helpstr);
  }
  
  patch.block_to_patch_nb = 1;

  if(remove) {
    patch.action = REMOVE_BLOCK;
  }
  else {
    patch.action = ADD_BLOCK;
  }
}

void RCparser_parse ( char * infilename, char * ModuleName ) {
  FILE *infile;
  int buffer_size;

  infile = fopen(infilename,"r");
  if(infile == NULL) {
    utilsPrintFatal(RCPARSER_ERROR_CANNOT_READ_FILE, infilename, utilsPrintSystemError());
  }

  if( (buffer_size = getFileSize(infilename,ModuleName)) == -1) {
    utilsPrintFatal(RCPARSER_ERROR_UNABLE_TO_GET_FILE_SIZE, infilename);
  }

  if( ! getExtensionSwitch( infile, buffer_size) ) {
    utilsPrintFatal(RCPARSER_ERROR_FAIL_TO_PATCH_FILE);
  }

  fclose(infile);
}

void RCparser_patch ( char * outfilename ) {
  FILE *outfile;

  if(strcmp(outfilename,"stdout") == 0) {
    outfile = stdout;
  }
  else {
    outfile = fopen(outfilename,"w");
    if(outfile == NULL) {
      utilsPrintFatal(RCPARSER_ERROR_CANNOT_WRITE_FILE, outfilename, utilsPrintSystemError());
    }
  }


  if( ! applyPatch( outfile ) ) {
    utilsPrintFatal(RCPARSER_ERROR_FAIL_TO_PATCH_FILE);
  }

  if(outfile != stdout) {
    fclose(outfile);
  }
}

int RCparser_getextnr ( void ) {
   return patch.current_block_nb_used;
}

RCparser_ExtensionInfoT * RCparser_getextinfo ( int index, char * ModuleName ) {
   static RCparser_ExtensionInfoT extinfo;
   
   ASSERT (index<=patch.current_block_nb_used, ModuleName, "Unable to get extension information");
   
   if (patch.rctype == RCPARSER_OPEN64) {
      extinfo.Name = patch.found_blocks[index].header_start;
      Getextinfo_open64(patch.found_blocks[index].body_start,&extinfo);
   } else {
      utilsPrintFatal(RCPARSER_ERROR_GETEXTINFO_NOT_SUPPORTED, patch.rctype==RCPARSER_FLEXCC?"FlexCC":"???");
   }
   return &extinfo;
}

#define PRINT_EXTINFO(stream,str,var) fprintf(stream,"%s = %s\n",str,var)
void RCparser_printextinfo ( RCparser_ExtensionInfoT * extinfo ) {
   fprintf(stdout,"ExtName [%s]\n",extinfo->Name);
   PRINT_EXTINFO(stdout,"   ARCHITECTURE ",extinfo->Architecture);
   PRINT_EXTINFO(stdout,"   EXTHWTYPE    ",extinfo->ExtHwType);
   PRINT_EXTINFO(stdout,"   O64LIBPATH   ",extinfo->CmpLibPath);
   PRINT_EXTINFO(stdout,"   O64LIBNAME   ",extinfo->CmpLibName);
   PRINT_EXTINFO(stdout,"   O64TARG      ",extinfo->CmpTargOpt);
   PRINT_EXTINFO(stdout,"   ASMLIBPATH   ",extinfo->AsmLibPath);
   PRINT_EXTINFO(stdout,"   ASMLIBNAME   ",extinfo->AsmLibName);
   PRINT_EXTINFO(stdout,"   LDLIBPATH    ",extinfo->LdLibPath);
   PRINT_EXTINFO(stdout,"   LDLIBNAME    ",extinfo->LdLibName);
   PRINT_EXTINFO(stdout,"   DISASMLIBPATH",extinfo->DisasmLibPath);
   PRINT_EXTINFO(stdout,"   DISASMLIBNAME",extinfo->DisasmLibName);
   PRINT_EXTINFO(stdout,"   EXTLIBPATH   ",extinfo->ToolsetLibPath);
   PRINT_EXTINFO(stdout,"   EXTLIBNAME   ",extinfo->ToolsetLibName);
   PRINT_EXTINFO(stdout,"   DEFINES      ",extinfo->Defines);
   PRINT_EXTINFO(stdout,"   INCLUDES     ",extinfo->Includes);
   PRINT_EXTINFO(stdout,"   XLIBS        ",extinfo->XLibs);
   PRINT_EXTINFO(stdout,"   HELP         ",extinfo->HelpMsg);
}
