##
## Makefile.am for HTML documents
##

SRC = Intro.in Tutorial.in reference.txt standard.css rgb.txt
PERL = perl
MK_HTML = make_html.pl 
RM = rm -f
CP = cp
MKDIR = mkdir -p

all: Intro.html
	$(PERL) $(MK_HTML)

install: Intro.html standard.css
	$(MKDIR) $(prefix)/html $(prefix)/html/code $(prefix)/html/img
	$(CP) *.html standard.css $(prefix)/html
	$(CP) -r code/* $(prefix)/html/code
	$(CP) -r img/* $(prefix)/html/img

Intro.html: $(SRC) $(MK_HTML) standard.css
	$(PERL) $(MK_HTML)

clean:
	$(RM) *.html
