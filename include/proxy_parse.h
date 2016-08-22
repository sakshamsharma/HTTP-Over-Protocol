/*
 * proxy_parse.h -- a HTTP Request Parsing Library.
 *
 * Written by: Matvey Arye
 * For: COS 518 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <ctype.h>

#ifndef PROXY_PARSE
#define PROXY_PARSE

#define DEBUG 1

/* 
   ParsedRequest objects are created from parsing a buffer containing a HTTP
   request. The request buffer consists of a request line followed by a number
   of headers. Request line fields such as method, protocol etc. are stored
   explicitly. Headers such as 'Content-Length' and their values are maintained
   in a linked list. Each node in this list is a ParsedHeader and contains a
   key-value pair.

   The buf and buflen fields are used internally to maintain the parsed request
   line.
 */
struct ParsedRequest {
     char *method; 
     char *protocol; 
     char *host; 
     char *port; 
     char *path;
     char *version;
     char *buf;
     size_t buflen;
     struct ParsedHeader *headers;
     size_t headersused;
     size_t headerslen;
};

/* 
   ParsedHeader: any header after the request line is a key-value pair with the
   format "key:value\r\n" and is maintained in the ParsedHeader linked list
   within ParsedRequest
*/
struct ParsedHeader {
     char * key;
     size_t keylen;
     char * value;
     size_t valuelen;
};


/* Create an empty parsing object to be used exactly once for parsing a single
 * request buffer */
struct ParsedRequest* ParsedRequest_create();

/* Parse the request buffer in buf given that buf is of length buflen */
int ParsedRequest_parse(struct ParsedRequest * parse, const char *buf,
			int buflen);

/* Destroy the parsing object. */
void ParsedRequest_destroy(struct ParsedRequest *pr);

/* 
   Retrieve the entire buffer from a parsed request object. buf must be an
   allocated buffer of size buflen, with enough space to write the request
   line, headers and the trailing \r\n. buf will not be NUL terminated by
   unparse().
 */
int ParsedRequest_unparse(struct ParsedRequest *pr, char *buf, 
			  size_t buflen);

/* 
   Retrieve the entire buffer with the exception of request line from a parsed
   request object. buf must be an allocated buffer of size buflen, with enough
   space to write the headers and the trailing \r\n. buf will not be NUL
   terminated by unparse(). If there are no headers, the trailing \r\n is
   unparsed.
 */
int ParsedRequest_unparse_headers(struct ParsedRequest *pr, char *buf, 
				  size_t buflen);

/* Total length including request line, headers and the trailing \r\n*/
size_t ParsedRequest_totalLen(struct ParsedRequest *pr);

/* Length including headers, if any, and the trailing \r\n but excluding the
 * request line. 
 */
size_t ParsedHeader_headersLen(struct ParsedRequest *pr);

/* Set, get, and remove null-terminated header keys and values */
int ParsedHeader_set(struct ParsedRequest *pr, const char * key, 
		      const char * value);
struct ParsedHeader* ParsedHeader_get(struct ParsedRequest *pr, 
				      const char * key);
int ParsedHeader_remove (struct ParsedRequest *pr, const char * key);

/* debug() prints out debugging info if DEBUG is set to 1 */
void debug(const char * format, ...);

/* Example usage:

   const char *c = 
   "GET http://www.google.com:80/index.html/ HTTP/1.0\r\nContent-Length:"
   " 80\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\n\r\n";
   
   int len = strlen(c); 
   //Create a ParsedRequest to use. This ParsedRequest
   //is dynamically allocated.
   ParsedRequest *req = ParsedRequest_create();
   if (ParsedRequest_parse(req, c, len) < 0) {
       printf("parse failed\n");
       return -1;
   }

   printf("Method:%s\n", req->method);
   printf("Host:%s\n", req->host);

   // Turn ParsedRequest into a string. 
   // Friendly reminder: Be sure that you need to
   // dynamically allocate string and if you
   // do, remember to free it when you are done.
   // (Dynamic allocation wasn't necessary here,
   // but it was used as an example.)
   int rlen = ParsedRequest_totalLen(req);
   char *b = (char *)malloc(rlen+1);
   if (ParsedRequest_unparse(req, b, rlen) < 0) {
      printf("unparse failed\n");
      return -1;
   }
   b[rlen]='\0';
   // print out b for text request 
   free(b);
  

   // Turn the headers from the request into a string.
   rlen = ParsedHeader_headersLen(req);
   char buf[rlen+1];
   if (ParsedRequest_unparse_headers(req, buf, rlen) < 0) {
      printf("unparse failed\n");
      return -1;
   }
   buf[rlen] ='\0';
   //print out buf for text headers only 

   // Get a specific header (key) from the headers. A key is a header field 
   // such as "If-Modified-Since" which is followed by ":"
   struct ParsedHeader *r = ParsedHeader_get(req, "If-Modified-Since");
   printf("Modified value: %s\n", r->value);
   
   // Remove a specific header by name. In this case remove
   // the "If-Modified-Since" header. 
   if (ParsedHeader_remove(req, "If-Modified-Since") < 0){
      printf("remove header key not work\n");
     return -1;
   }

   // Set a specific header (key) to a value. In this case,
   //we set the "Last-Modified" key to be set to have as 
   //value  a date in February 2014 
   
    if (ParsedHeader_set(req, "Last-Modified", " Wed, 12 Feb 2014 12:43:31 GMT") < 0){
     printf("set header key not work\n");
     return -1;

    }

   // Check the modified Header key value pair
    r = ParsedHeader_get(req, "Last-Modified");
    printf("Last-Modified value: %s\n", r->value);

   // Call destroy on any ParsedRequests that you
   // create once you are done using them. This will
   // free memory dynamically allocated by the proxy_parse library. 
   ParsedRequest_destroy(req);
*/

#endif

