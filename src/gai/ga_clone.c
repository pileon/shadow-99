#include	"gai_hdr.h"

/*
 * Clone a new addrinfo structure from an existing one.
 */

/* include ga_clone */
struct addrinfo *
ga_clone(struct addrinfo *ai)
{
	struct addrinfo	*newai;

	if ( (newai = (struct addrinfo *)
		  calloc(1, sizeof(struct addrinfo))) == NULL)
		return(NULL);

	newai->ai_next = ai->ai_next;
	ai->ai_next = newai;

	newai->ai_flags = 0;				/* make sure AI_CLONE is off */
	newai->ai_family = ai->ai_family;
	newai->ai_socktype = ai->ai_socktype;
	newai->ai_protocol = ai->ai_protocol;
	newai->ai_canonname = NULL;
	newai->ai_addrlen = ai->ai_addrlen;
	if ( (newai->ai_addr = (struct sockaddr *) malloc(ai->ai_addrlen)) == NULL)
		return(NULL);
	memcpy(newai->ai_addr, ai->ai_addr, ai->ai_addrlen);

	return(newai);
}
/* end ga_clone */
