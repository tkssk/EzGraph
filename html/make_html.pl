#!/usr/bin/perl

$ref_file = "reference.txt";
$rgb_file = "rgb.txt";

if(!open(FI, $ref_file)){
	print "Cannot open $ref_file.\n";
	exit 1;
}

$idx = 0;
$mode = 0;
while(<FI>){
	if(/^#/){
		next;
	}
	if(/^F\s+(.*)/){
		$f .= $1;
	}
	if(/^T\s+(.*)/){
		$t .= $1;
	}
	if(/^S\s+(.*)/){
		$s .= $1;
	}
	if(/^D\s+(.*)/){
		if($mode == 1){
			$d .= "</pre></code><p>";
		}
		if($d eq ""){
			$d = "<p>";
		}
		$tmp = $1;
		while($tmp =~ /^(.*)\?(.*):(\w+?)\?(.*)$/){
			$tmp = sprintf("%s<a href=\"%s.html\">%s</a>%s",
				$1, $3, $2, $4);
		}
		while($tmp =~ /^(.*)\?(\w+?)\?(.*)$/){
			$tmp = sprintf("%s<a href=\"%s.html\">%s</a>%s",
				$1, $2, $2, $3);
		}
		$d .= $tmp;
		$mode = 0;
	}
	if(/^\=\s*$/){
		if($mode == 1){
			$d .= "</pre></code>";
		}
		$d .= "\n<p>";
		$mode = 0;
	}
	if(/^C\s*$/){
		if($mode == 0){
			$d .= "\n<code><pre>";
		}
		$d .= "\n";
		$mode = 1;

	}
	elsif(/^C\ (.*)/){
		if($mode == 0){
			$d .= "\n<code><pre>";
		}
		$tmp = $1;
		$tmp =~ s/\ /&nbsp;/g;
		$tmp =~ s/\t/&nbsp;&nbsp;&nbsp;&nbsp;/g;
		$d .= $tmp;
		$d .= "\n";
		$mode = 1;
	}
	if(/^\$\s*$/){
		if($mode == 1){
			$d .= "</pre></code>\n";
		}

		push(@f, $f);
		push(@t, $t);
		push(@s, $s);
		push(@d, $d);

		if($f =~ /(\w+)\(/){
			$name = $1;
		}else{	
			print "System error.\n";
			exit 1;
		}
		push(@n, $name);

		if(grep(/$t/, @tl) == ()){
			push(@tl, $t);
		}

		$refidx{$name} = $idx;
		$idx++;

		undef $f;
		undef $t;
		undef $s;
		undef $d;
		undef $name;
		$mode = 0;
	}
}
close(FI);

&In2Html("Intro");
&In2Html("Tutorial");
&In2Html("TutBasic");
&In2Html("TutComp");
&In2Html("TutEvent");
&In2Html("TutDoubleBuf");
&In2Html("TutXPM");
&In2Html("TutKey");
&In2Html("TutNet");
&In2Html("KeyTable");
&In2Html("MML");
&FuncList();
&RefList();
&TypeList();
&ColorTable();
&Func();
if(! -e "index.html"){
	system("ln", "-s", "Intro.html", "index.html");
}
exit 0;

##################################################################
sub In2Html {
	my($file) = @_;
	open(FO, ">$file.html") || die("Cannot create $file.html\n");
	open(FI, "$file.in") || die("Cannot open $file.in\n");
	&head();
	#@html = <FI>;
	#print FO @html;
	while(<FI>){
		while(/^(.*)\?(.*):(\w+?)\?(.*)$/){
			$_ = sprintf("%s<a href=\"%s.html\">%s</a>%s",
				$1, $3, $2, $4);
		}
		while(/^(.*)\?(\w+?)\?(.*)$/){
			$_ = sprintf("%s<a href=\"%s.html\">%s</a>%s",
				$1, $2, $2, $3);
		}
		print FO $_;
	}
	&foot();
	close(FI);
	close(FO);
}

##################################################################
sub FuncList {
	open(FO, ">FuncList.html") || die("Cannot create FuncList.html\n");
	&head();
	print FO "<h2><a name=\"FuncList\">機能一覧</a></h2>\n";
	print FO "<ol>\n";
	for($i=0; $i<=$#f; $i++){
		printf(FO "<li><a href=\"%s.html\">%s</a>\n", $n[$i], $s[$i]);
	}
	print FO "</ol>\n";
	&foot();
	close(FO);
}

##################################################################
sub RefList {
	open(FO, ">RefList.html") || die("Cannot create RefList.html\n");
	&head();
	print FO "<h2><a name=\"RefList\">関数名一覧（ABC順）</a></h2>\n";
	print FO "<ol>\n";
	@sorted = sort(@n);
	for($i=0; $i<=$#n; $i++){
		printf(FO "<li><a href=\"%s.html\">%s</a>\n",
			$sorted[$i], $f[$refidx{$sorted[$i]}]);
	}
	print FO "</ol>\n";
	&foot();
	close(FO);
}

##################################################################
sub TypeList {
	open(FO, ">TypeList.html") || die("Cannot create TypeList.html\n");
	&head();
	print FO "<h2><a name=\"RefList\">機能別一覧</a></h2>\n";
	print FO "<ol>\n";
	for($j=0; $j<=$#tl; $j++){
		printf(FO "<li>%s\n", $tl[$j]);
		print FO "<ul>\n";
		for($i=0; $i<=$#f; $i++){
			if($t[$i] eq $tl[$j]){
				printf(FO "<li><a href=\"%s.html\">%s</a>\n", $n[$i], $s[$i]);
			}
		}
		print FO "</ul>\n";
	}
	print FO "</ol>\n";
	&foot();
	close(FO);
}

##################################################################
sub Func {
	for($i=0; $i<=$#f; $i++){
		$html = sprintf("%s.html", $n[$i]);
		open(FO, ">$html") || die("Cannot create $html\n");
		&head();
		print FO "<h2>説明</h2>\n";
		print FO "<div class=\"body\">\n";
		print FO "<dl>\n";

		print FO "<dt>名前</dt>\n";
		printf(FO "<dd>%s</dd>\n", $n[$i]);
		print FO "<br>\n";

		print FO "<dt>機能</dt>\n";
		printf(FO "<dd>%s</dd>\n", $s[$i]);
		print FO "<br>\n";

		print FO "<dt>プロトタイプ</dt>\n";
		printf(FO "<dd>%s</dd>\n", $f[$i]);
		print FO "<br>\n";

		print FO "<dt>関数の説明</dt>\n";
		printf(FO "<dd>%s</dd>\n", $d[$i]);

		print FO "</dl>\n";
		print FO "</div>\n";
		&foot();
		close(FO);
	}
}

##################################################################
sub ColorTable {
	open(FO, ">ColorTable.html") || die("Cannot create ColorTable.html\n");
	&head();
	print FO "<h2><a name=\"FuncList\">色見本</a></h2>\n";
	if(!open(FI, $rgb_file)){
		print "Cannot open $rgb_file<br>\n";
		exit 1;
	}
print FO <<END_OF_LINE;
<table border=1>
<tr>
	<td>色名</td><td>赤(R)</td><td>緑(G)</td><td>青(B)</td><td>サンプル</td>
</tr>
END_OF_LINE

	while(<FI>){
		while(s/^\ //){;}
		($r, $g, $b, $name) = split(/[ \t\n]+/);
		printf(FO "<tr><td>%s</td><td>%d</td><td>%d</td><td>%d</td>".
			"<td bgcolor=\"#%02x%02x%02x\"><br></td></tr>\n",
			$name, $r, $g, $b, $r, $g, $b);
	}
	print FO "</table>\n";
	&foot();
	close(FO);
}

##################################################################
sub head{
	print FO <<END_OF_LINE;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
  "http://www.w3.org/TR/html14/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=EUC-JP">
<link rel="stylesheet" type="text/css" href="standard.css">
<title>EzGraph : Easy Graphic Library</title>
</head>
<body>
<h1>EzGraph : Eazy Graphic Library</h1>
<table width="100%">
<tr>
<td><a href="Intro.html">概要</a></td>
<td><a href="Tutorial.html">チュートリアル</a></td>
<td><a href="FuncList.html">機能一覧</a></td>
<td><a href="RefList.html">関数名一覧</a></td>
<td><a href="TypeList.html">機能別一覧</a></td>
<td><a href="ColorTable.html">色見本</a></td>
<td><a href="MML.html">MMLの書き方</a></td>
</tr>
</table>
END_OF_LINE
}

##################################################################
sub foot {
	print FO <<END_OF_LINE;
<hr>
<address>
Takahiro SASAKI<br>
E-mail: sasaki at arch.info.mie-u.ac.jp
</address>
</body>
</html>
END_OF_LINE

}
