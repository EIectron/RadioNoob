#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

const char data[] = {
0xEA,	  // Destination
0x21, 	// Len
0x2B, 	// Frame Type
0xEA, 	// extended packet dest
0xEE, 	// extended packet src
0x03, 	// Config field index
0x00, 	// Chunk
0x00, 	// Parent
0x09, 	// Field Type
0x42, 0x54, 0x20, 0x54, 0x65, 0x6C, 0x65, 0x6D, 0x65, 0x74, 0x72, 0x79, 0x00, // BT Telemetry
0x4F, 0x66, 0x66, 0x3B, 0x4F, 0x6E, 0x00, // Off;On
0x00, 	// Field Value
0x00, 	// Minimum Value
0x01, 	// Maximum Value
0x00, 	// Default Value
0x00, 	// Unit
0x4C	// CRC
};

typedef struct {
    char label[256];
    char options[256];
    char *name;
    int type;
    int value;
    int minValue;
    int maxValue;
    char units[256];
} MenuItem;

MenuItem menu;


char recv_param_buffer[CRSF_MAX_CHUNKS * CRSF_MAX_CHUNK_SIZE];
char *recv_param_ptr;
u8 next_param;
u8 next_chunk;
u8 device_idx = 12;
u8 chunk;
u8 params_loaded = 0;
u8 update = 0;
crsf_param_t crsf_params[CRSF_MAX_PARAMS];


static void crsfdevice_init() {
    next_param = 1;
    next_chunk = 0;
    recv_param_ptr = recv_param_buffer;
    memset(crsf_params, 0, sizeof(crsf_params));
}

u32 millis()
{
  return 1000;
}


void parse_bytes(enum data_type type, char **buffer, char *dest)
{
  switch (type)
  {
  case UINT8:
    *(u8 *)dest = (u8)(*buffer)[0];
    *buffer += 1;
    break;
  case INT8:
    *(s8 *)dest = (s8)(*buffer)[0];
    *buffer += 1;
    break;
  case UINT16:
    *(u16 *)dest = (u16)(((*buffer)[0] << 8) | (*buffer)[1]);
    *buffer += 2;
    break;
  case INT16:
    *(s16 *)dest = (s16)(((*buffer)[0] << 8) | (*buffer)[1]);
    *buffer += 2;
    break;
  case FLOAT:
    *(s32 *)dest = (s32)(((*buffer)[0] << 24) | ((*buffer)[1] << 16) | ((*buffer)[2] << 8) | (*buffer)[3]);
    *buffer += 4;
    break;
  default:
    break;
  }
}

void add_param(u8 *buffer, u8 num_bytes)
{

    char *param_values;

    memcpy(recv_param_ptr, buffer+5, num_bytes-5);
    recv_param_ptr += num_bytes - 5;

    recv_param_ptr = recv_param_buffer;

    u8 param_id = buffer[5];

    crsf_params[param_id].device = 0;
    crsf_params[param_id].id = *recv_param_ptr++;
    crsf_params[param_id].parent = *recv_param_ptr++;
    crsf_params[param_id].chunk = *recv_param_ptr++;
    crsf_params[param_id].type = *recv_param_ptr & 0x7f;

    update = crsf_params[param_id].id == param_id;

    if(!update)
    {
      crsf_params[param_id].hidden = *recv_param_ptr++ & 0x80;
      crsf_params[param_id].name = (char *)malloc(strlen(recv_param_ptr) + 1);
      strncpy(crsf_params[param_id].name, (const char *)recv_param_ptr, strlen(recv_param_ptr) + 1);
      recv_param_ptr += strlen(recv_param_ptr) + 1;
    }
    else
    {
      if (crsf_params[param_id].hidden != (*recv_param_ptr & 0x80))
            params_loaded = 0;   // if item becomes hidden others may also, so reload all params
        crsf_params[param_id].hidden = *recv_param_ptr++ & 0x80;
        recv_param_ptr += strlen(recv_param_ptr) + 1;
    }

    

    u8 count;

    switch(crsf_params[param_id].type)
    {

       case UINT8:
       case INT8:
       case UINT16:
       case INT16:
       case FLOAT:
        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].value);
        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].min_value);
        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].max_value);
        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].default_value);
        if (crsf_params[param_id].type == FLOAT) {
            parse_bytes(UINT8, &recv_param_ptr, (char *)&crsf_params[param_id].u.point);
            parse_bytes(FLOAT, &recv_param_ptr, (char *)&crsf_params[param_id].step);
        } else if (*recv_param_ptr) {
            const u8 length = strlen(recv_param_ptr) + 1;
            if (!update) crsf_params[param_id].s.unit = malloc(length);
            strncpy(crsf_params[param_id].s.unit, (const char *)recv_param_ptr, length);
        }
       break;
       case TEXT_SELECTION:
            param_values = (char *)malloc(strlen(recv_param_ptr) + 1);
            strncpy(param_values, (const char *)recv_param_ptr, strlen(recv_param_ptr) + 1);
            recv_param_ptr += strlen(recv_param_ptr) + 1;
             // put null between selection options
            // find max choice string length to adjust textselectplate size
            char *start = (char *)param_values;
            count = 0;
            for (char *p = (char *)param_values; *p; p++)
            {
                if (*p == ';')
                {
                    u8 len = (strlen(start) - strlen(p));
                    crsf_params[param_id].options[count] = (char *)malloc(len + 1);
                    strncpy(crsf_params[param_id].options[count], start, len + 1);
                    crsf_params[param_id].options[count][len] = '\0';

                    start = p + 1;
                    count += 1;
                }
            }

            int len = strlen(start);
            crsf_params[param_id].options[count] = (char *)malloc(len + 1);
            strncpy(crsf_params[param_id].options[count], start, len + 1);

            // free(start);

            crsf_params[param_id].count = count;

            buffer = &buffer[buffer[1] - 4];

            parse_bytes(UINT8,  (char *)&buffer, (char *)&crsf_params[param_id].status);
            parse_bytes(UINT8,  (char *)&buffer, (char *)&crsf_params[param_id].min_value);
            parse_bytes(UINT8,  (char *)&buffer, (char *)&crsf_params[param_id].max_value); // don't use incorrect parameter->max_value
            parse_bytes(UINT8,  (char *)&buffer, (char *)&crsf_params[param_id].default_value);
        break;

        case INFO:
        if (!update) 
        {
            const u8 length = strlen(recv_param_ptr) + 1;
            crsf_params[param_id].value = malloc(length);
            strncpy(crsf_params[param_id].value, (const char *)recv_param_ptr, length);
        }

        case STRING:
        {
            const char *value;
            value = recv_param_ptr;
            recv_param_ptr += strlen(value) + 1;
            parse_bytes(UINT8, &recv_param_ptr, &crsf_params[param_id].u.string_max_len);

            // No string re-sizing so allocate max length for value
            if (!update)crsf_params[param_id].value = malloc(crsf_params[param_id].u.string_max_len+1);
            strncpy(crsf_params[param_id].value, value, crsf_params[param_id].u.string_max_len+1);
        }

        case COMMAND:
          parse_bytes(UINT8, &recv_param_ptr, (char *)&crsf_params[param_id].u.status);
          parse_bytes(UINT8, &recv_param_ptr, (char *)&crsf_params[param_id].timeout);
          if (!update) crsf_params[param_id].s.info = malloc(40);
          strncpy(crsf_params[param_id].s.info, (const char *)recv_param_ptr, 40);

          command.param = crsf_params;
          command.time = 0;
          switch (crsf_params[param_id].u.status) {
          case PROGRESS:
              command.time = millis();
              // FALLTHROUGH
          case CONFIRMATION_NEEDED:
              command.dialog = 1;
              break;
          case READY:
              command.dialog = 2;
              break;
          }
        break;
        case FOLDER:
          printf("[i] Folder Parameter does not support right now:/\n");
          return;
        break;
        case OUT_OF_RANGE:
        default:
            break;
    }


    // printf("\rDevice: %X\n \
    //         \rId: %X\n    \
    //         \rParent: %d\n \
    //         \rType: %X\n \
    //         \rName: %s\n \
    //         \rValue: %s\n\n",
    //         crsf_params[param_id].device,
    //         crsf_params[param_id].id,
    //         crsf_params[param_id].parent,
    //         crsf_params[param_id].type,
    //         crsf_params[param_id].name,
    //         crsf_params[param_id].status);

    // free(param_values);
        
    printf("Options; \n");
    for(int i = 0; i<=crsf_params[param_id].count; i++)
    {
        printf("%s\n", crsf_params[param_id].options[i]);
    }

}

int main(void)
{
    crsfdevice_init();
    add_param((u8 *)param2, sizeof(param2));
	return 0;
}
