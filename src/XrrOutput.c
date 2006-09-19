/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <X11/Xlib.h>
/* we need to be able to manipulate the Display structure on events */
#include <X11/Xlibint.h>
#include <X11/extensions/render.h>
#include <X11/extensions/Xrender.h>
#include "Xrandrint.h"

XRROutputInfo *
XRRGetOutputInfo (Display *dpy, XRRScreenResources *resources, RROutput output)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetOutputInfoReply	rep;
    xRRGetOutputInfoReq		*req;
    int				nbytes, nbytesRead, rbytes;
    int				i;
    xRRQueryVersionReq		*vreq;
    XRROutputInfo		*xoi;

    RRCheckExtension (dpy, info, 0);

    GetReq (RRGetOutputInfo, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetOutputInfo;
    req->output = output;
    req->configTimestamp = resources->configTimestamp;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	SyncHandle ();
	return NULL;
    }

    nbytes = (long) rep.length << 2;

    nbytesRead = (long) (rep.nCrtcs * 4 +
			 rep.nModes * 4 +
			 rep.nClones * 4 +
			 ((rep.nameLength + 3) & ~3));

    /* 
     * first we must compute how much space to allocate for 
     * randr library's use; we'll allocate the structures in a single
     * allocation, on cleanlyness grounds.
     */

    rbytes = (sizeof (XRROutputInfo) +
	      rep.nCrtcs * sizeof (RRCrtc) +
	      rep.nModes * sizeof (RRMode) +
	      rep.nClones * sizeof (RROutput) +
	      rep.nameLength + 1);	    /* '\0' terminate name */

    xoi = (XRROutputInfo *) Xmalloc(rbytes);
    if (xoi == NULL) {
	_XEatData (dpy, (unsigned long) nbytes);
	SyncHandle ();
	return NULL;
    }

    xoi->timestamp = rep.timestamp;
    xoi->crtc = rep.crtc;
    xoi->connection = rep.connection;
    xoi->subpixel_order = rep.subpixelOrder;
    xoi->ncrtc = rep.nCrtcs;
    xoi->crtcs = (RRCrtc *) (xoi + 1);
    xoi->nmode = rep.nModes;
    xoi->modes = (RRMode *) (xoi->crtcs + rep.nCrtcs);
    xoi->nclone = rep.nClones;
    xoi->clones = (RROutput *) (xoi->modes + rep.nModes);
    xoi->name = (char *) (xoi->clones + rep.nClones);

    _XRead32 (dpy, xoi->crtcs, rep.nCrtcs << 2);
    _XRead32 (dpy, xoi->modes, rep.nModes << 2);
    _XRead32 (dpy, xoi->clones, rep.nClones << 2);
    
    /*
     * Read name and '\0' terminate
     */
    _XRead (dpy, xoi->name, rep.nameLength);
    xoi->name[rep.nameLength] = '\0';
    
    /*
     * Skip any extra data
     */
    if (nbytes > nbytesRead)
	_XEatData (dpy, (unsigned long) (nbytes - nbytesRead));
    
    return (XRROutputInfo *) xoi;
}

void
XRRFreeOutputInfo (XRROutputInfo *outputInfo)
{
    Xfree (outputInfo);
}

Atom *
XRRListOutputProperties (Display *dpy, RROutput output, int *nprop)
{
}

void
XRRChangeOutputProperty (Display *dpy, RROutput output,
			 Atom property, Atom type,
			 int format, int mode,
			 _Xconst unsigned char *data, int nelements)
{
}

void
XRRDeleteOutputProperty (Display *dpy, RROutput output, Atom property)
{
}

int
XRRGetOutputProperty (Display *dpy, RROutput output,
		      Atom property, long offset, long length,
		      Bool delete, Atom req_type, 
		      Atom *actual_type, int *actual_format,
		      unsigned long *nitems, unsigned long *bytes_after,
		      unsigned char **prop)
{
}