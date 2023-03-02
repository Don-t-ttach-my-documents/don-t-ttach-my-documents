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
    /* Pas assez de mémoire ! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), data, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = '.';

  return realsize;
}

static size_t send_data_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *wt = (struct WriteThis *)userp;
  size_t buffer_size = size * nmemb;

  if (wt->sizeleft)
  {
    /* copie autant de données que possible de la source à la destination */
    size_t copy_this_much = wt->sizeleft;
    if (copy_this_much > buffer_size)
      copy_this_much = buffer_size;
    memcpy(dest, wt->readptr, copy_this_much);

    wt->readptr += copy_this_much;
    wt->sizeleft -= copy_this_much;
    return copy_this_much; /* on a copié tant */
  }

  return 0; /* plus de données à délivrer */
}

int sendBodyToParsing(char *body, size_t lenBody, struct MemoryStruct *parsed, char* sender)
{
  // Changer l'adresse vers le service de parsing
  static const char url[27] = "http://parsing:3201/upload";

  CURL *curl;
  CURLcode res;

  struct WriteThis wt;
  wt.readptr = body;
  wt.sizeleft = lenBody;

  /* curl handle */
  curl = curl_easy_init();
  if (curl)
  {
    /* Ajout de l'url */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Requete http POST */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    /* Ajout de notre fonction de lecture */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, send_data_callback);
    
    /* Pointeur à passer à notre fonction de lecture */
    curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

    /* Taille du corps de la requete POST */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);

    /* envoie des données  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_data);

    /* récupération des données */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)parsed);

    /* 1 pour avoir un debug verbeux, 0L sinon */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    /* ajout du mail de l'expéditaire dans les headers */
    struct curl_slist *header = NULL;
    char* initSender = malloc(strlen(sender)+ strlen("Sender: ") +2);
    strcpy(initSender, "Sender: ");
    header = curl_slist_append(header, strcat(initSender, sender));
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

    /* Envoie de la requete, res retourne le code d'erreur */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* Nettoyage */
    curl_easy_cleanup(curl);
    curl_slist_free_all(header);
    free(initSender);
    return PARSING_OK;
  }
  return PARSING_ERROR;
}