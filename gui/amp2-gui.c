/*
 *
 * amp2-gui.c - minimal AROS GUI launcher for AMP2
 *
 * Opens a small window with a "Play File..." button that pops an ASL file
 * requester and then runs  SYS:Extras/Audio/AMP2/AMP "<file>" WINDOW.
 *
 * Uses AROS auto-libraries (-lamiga), like the rest of the AMP2 port.
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include <string.h>
#include <stdio.h>

#define AMP_CMD "SYS:Extras/Audio/AMP2/AMP"
#define AMP_DRAWER "SYS:Extras/Audio/AMP2"
#define AMP_TESTS "SYS:Extras/Audio/AMP2/tests"

static char status_text[160];

static void set_status(struct Gadget *gt, const char *s)
{
  strncpy(status_text, s, sizeof(status_text) - 1);
  status_text[sizeof(status_text) - 1] = 0;
  if (gt != NULL) {
    GT_SetGadgetAttrs(gt, NULL, NULL, GTTX_Text, status_text, TAG_DONE);
  }
}

int main(void)
{
  struct Screen *screen = NULL;
  struct Window *win = NULL;
  struct Gadget *glist = NULL, *gad = NULL, *g_play = NULL, *g_status = NULL;
  struct NewGadget ng;
  struct VisualInfo *vi = NULL;
  struct IntuiMessage *imsg;
  struct FileRequester *fr = NULL;
  struct TextAttr ta;
  BOOL done = FALSE;
  char path[320];
  char cmd[520];

  {
    int retry;
    for (retry = 0; retry < 100; retry++) {
      screen = LockPubScreen(NULL);
      if (screen != NULL) {
        break;
      }
      Delay(25);
    }
  }
  if (screen == NULL) {
    printf("amp2-gui: could not lock Workbench screen\n");
    return 1;
  }

  vi = GetVisualInfoA(screen, NULL);
  if (vi == NULL) {
    printf("amp2-gui: GetVisualInfoA failed\n");
    UnlockPubScreen(NULL, screen);
    return 1;
  }

  /* GadTools needs a TextAttr; use the screen's font. */
  ta.ta_Name = (STRPTR)"topaz.font";
  ta.ta_YSize = 8;
  ta.ta_Style = 0;
  ta.ta_Flags = 0;

  memset(&ng, 0, sizeof(ng));
  ng.ng_TextAttr = (screen->Font != NULL) ? screen->Font : &ta;
  ng.ng_VisualInfo = vi;

  glist = CreateContext(&glist);
  gad = glist;

  /* Status text line */
  strcpy(status_text, "Select a file to play");
  ng.ng_LeftEdge = 10;
  ng.ng_TopEdge = 10;
  ng.ng_Width = 340;
  ng.ng_Height = 14;
  ng.ng_GadgetText = (STRPTR)"";
  ng.ng_Flags = 0;
  g_status = CreateGadget(TEXT_KIND, gad, &ng,
                          GTTX_Text, status_text,
                          GTTX_Border, TRUE,
                          TAG_DONE);
  gad = g_status;

  /* Play button */
  ng.ng_LeftEdge = 80;
  ng.ng_TopEdge = 38;
  ng.ng_Width = 110;
  ng.ng_Height = 20;
  ng.ng_GadgetText = "Play File...";
  ng.ng_Flags = PLACETEXT_IN;
  g_play = CreateGadget(BUTTON_KIND, gad, &ng,
                        GA_Text, "Play File...",
                        GA_RelVerify, TRUE,
                        TAG_DONE);
  gad = g_play;

  /* Quit button */
  ng.ng_LeftEdge = 205;
  ng.ng_TopEdge = 38;
  ng.ng_Width = 70;
  ng.ng_Height = 20;
  ng.ng_GadgetText = "Quit";
  gad = CreateGadget(BUTTON_KIND, gad, &ng,
                     GA_Text, "Quit",
                     GA_RelVerify, TRUE,
                     TAG_DONE);
  {
    struct Gadget *g_quit = gad;

    win = OpenWindowTags(NULL,
                         WA_Title, "AMP2 Launcher",
                         WA_Left, 80, WA_Top, 60,
                         WA_Width, 360, WA_Height, 80,
                         WA_CustomScreen, screen,
                         WA_Gadgets, glist,
                         WA_Flags, WFLG_DRAGBAR | WFLG_DEPTHGADGET |
                                   WFLG_CLOSEGADGET | WFLG_ACTIVATE,
                         WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_GADGETUP |
                                   IDCMP_VANILLAKEY,
                         TAG_DONE);
    if (win != NULL) {
      GT_RefreshWindow(win, NULL);

      while (!done) {
        WaitPort(win->UserPort);
        while ((imsg = (struct IntuiMessage *)GetMsg(win->UserPort)) != NULL) {
          ULONG iclass = imsg->Class;
          UWORD icode = imsg->Code;
          struct Gadget *igad = (struct Gadget *)imsg->IAddress;

          ReplyMsg((struct Message *)imsg);

          switch (iclass) {
            case IDCMP_CLOSEWINDOW:
              done = TRUE;
              break;

            case IDCMP_VANILLAKEY:
              if (icode == 27 || icode == 'q' || icode == 'Q') {
                done = TRUE;
              }
              break;

            case IDCMP_GADGETUP:
              if (igad == g_quit) {
                done = TRUE;
              } else if (igad == g_play) {
                if (fr == NULL) {
                  fr = AllocAslRequest(ASL_FileRequest, NULL);
                }
                if (fr != NULL) {
                  struct TagItem asltags[] = {
                    { ASLFR_TitleText,     (IPTR)"Choose a media file" },
                    { ASLFR_InitialDrawer, (IPTR)AMP_TESTS },
                    { ASLFR_InitialPattern,
                      (IPTR)"#?.(mpg|mpeg|avi|mov|qt|mp3|mp2|ac3|rm|m1v)" },
                    { ASLFR_DoPatterns,    TRUE },
                    { ASLFR_RejectIcons,   TRUE },
                    { ASLFR_SleepWindow,   TRUE },
                    { TAG_DONE,            0 }
                  };

                  AslRequest(fr, asltags);

                  if (fr->fr_File != NULL && fr->fr_File[0] != 0) {
                    path[0] = 0;
                    if (fr->fr_Drawer != NULL && fr->fr_Drawer[0] != 0) {
                      strncpy(path, fr->fr_Drawer, sizeof(path) - 1);
                      path[sizeof(path) - 1] = 0;
                      if (path[0] != 0) {
                        char last = path[strlen(path) - 1];
                        if (last != ':' && last != '/') {
                          strncat(path, "/", sizeof(path) - strlen(path) - 1);
                        }
                      }
                      strncat(path, fr->fr_File, sizeof(path) - strlen(path) - 1);
                    } else {
                      strncpy(path, fr->fr_File, sizeof(path) - 1);
                      path[sizeof(path) - 1] = 0;
                    }

                    snprintf(cmd, sizeof(cmd), AMP_CMD " \"%s\" WINDOW", path);
                    set_status(g_status, "Playing...");
                    GT_RefreshWindow(win, NULL);

                    Execute(cmd, NULL, NULL);

                    set_status(g_status, "Ready. Select a file to play");
                    GT_RefreshWindow(win, NULL);
                  }
                }
              }
              break;
          }
        }
      }

      if (fr != NULL) {
        FreeAslRequest(fr);
        fr = NULL;
      }

      CloseWindow(win);
      win = NULL;
    }
  }

  FreeGadgets(glist);
  FreeVisualInfo(vi);
  UnlockPubScreen(NULL, screen);

  return 0;
}
