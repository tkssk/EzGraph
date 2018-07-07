# EzGraph

EzGraph is simple graphic library for X11.

From this version, font rendering is processed by Xft.
So charactor code of source file is limitted to UTF-8.
EzGraph adopts iconv library to use EUC-JP, Shift JIS,
and UTF-8.  Default charactor code is UTF-8, so if you
want to use EUC-JP or Shift JIS, specify
	--enable-iconv=EUC-JP
		or
	--enable-iconv=SHIFT-JIS
at configuration.  Because of this modification,
two APIs are changed.
	EzSetFontName(<Font name of fontconfig>);
		Now font name is NOT XFLD.  Find desired font
		name by fc-list or fc-match commands.
	EzSetFOntSize(double size);
		Now <size> is changed from int to double.

Xdbe is now disabled by default.  If you want to use Xdbe,
specify
	--enable-Xdbe
at configuration.
