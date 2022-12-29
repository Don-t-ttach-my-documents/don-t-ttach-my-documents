#include "parsing.h"

#include <curl/curl.h>
#include <string.h>

int initLibcurl() {
  CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK){
    fprintf(stderr, "curl_global_init() failed: %s\n",
            curl_easy_strerror(res));
    curl_global_cleanup();
    return 1;
  }
  return 0;
}

static size_t receive_data(void *data, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), data, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = '.';

  return realsize;
}

//Envoie plusieurs requêtes en cas de pièce jointe trop lourdes
static size_t send_data_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *wt = (struct WriteThis *)userp;
  size_t buffer_size = size * nmemb;

  if (wt->sizeleft)
  {
    /* copy as much as possible from the source to the destination */
    size_t copy_this_much = wt->sizeleft;
    if (copy_this_much > buffer_size)
      copy_this_much = buffer_size;
    memcpy(dest, wt->readptr, copy_this_much);

    wt->readptr += copy_this_much;
    wt->sizeleft -= copy_this_much;
    return copy_this_much; /* we copied this many bytes */
  }

  return 0; /* no more data left to deliver */
}

int sendBodyToParsing(char *body, size_t lenBody, struct MemoryStruct *parsed, char* sender)
{
  static const char url[27] = "http://parsing:3201/upload";

  CURL *curl;
  CURLcode res;

  struct WriteThis wt;
  wt.readptr = body;
  wt.sizeleft = lenBody;

  /* get a curl handle */
  curl = curl_easy_init();
  if (curl)
  {
    /* First set the URL that is about to receive our POST. */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, send_data_callback);
    
    /* pointer to pass to our read/send function */
    curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

    /* Set the expected POST size. If you want to POST large amounts of data,
       consider CURLOPT_POSTFIELDSIZE_LARGE */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_data);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)parsed);

    /* get verbose debug output please */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    /* add mail of the sender in headers */
    struct curl_slist *header = NULL;
    char* initSender = malloc(strlen(sender)+ strlen("Sender: ") +2);
    strcpy(initSender, "Sender: ");
    header = curl_slist_append(header, strcat(initSender, sender));
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_slist_free_all(header);
    free(initSender);
    return PARSING_OK;
  }
  return PARSING_ERROR;
}