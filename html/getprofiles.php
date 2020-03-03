<!--- This file goes OUTSIDE of the profiles/ directory in your web directory
I.E. http://www.yourmud.com/getprofiles.php is where profiles are listed
and  http://www.yourmud.com/profiles/ is where all the profiles are stored
--->
<HTML>
<HEAD>
<style type="text/css">
body{font-family: Verdana; font-size: 10pt; color: #AA0000}
a{font-family: Verdana; font-size: 10pt; color: #AA0000}
a:hover{text-decoration: underline;}
.header{color: #770000; font-size: 14pt; text-align: center}
.ltable{border: 1px solid #990000; width: 150px;}
.letter{height: 20px; text-align: center; color: #808080; background-color: #220000; border-bottom: 1px solid #990000; font-weight: bold}
.info{color: #808080; padding: 5px;}
</style>
</HEAD>
<BODY bgcolor="#000000" background="back/homeback.jpg" bgproperties="fixed">
<div class="header">SW: FotE Player Info Database</div>
<hr noshade color="#770000" width="80%" align="center">
<table width="70%" align="center">
<tr>
<td width="75%" valign="top">

<?php 

function getNames ($dirName)
{ 
if ($d = @opendir("$dirName"))
{ 
 while($file=readdir($d)) 
 { 
 $files[]=$file; 
 } 
sort($files); 
reset ($files); 
$currval = "1";
while (list ($key, $val) = each ($files))
{ 
    if ($val != "." && $val != "..")
    { 
	$a = strpos($val, ".");
	if($a == 0)
	{
	  $a = strlen($val);
	}
	$newentry = substr($val, 0, $a);
	if(substr($newentry, 0, 1) != $currval)
	{
	  if($currval != "1")
	  {
	   print "
		</td>
		</tr>
		</table>
		 ";
	  }
	  $currval = substr($newentry, 0, 1);
	  if($currval == "N")
	  {
	    print "</td><td valign=\"top\">";
	  }
	  print "
		<br>
		<table class='ltable' cellpadding='0' cellspacing='0'>
		<tr>
		<td class='letter'>
		$currval
		</td>
		</tr>
		<tr>
		<td class='info' valign='top'>
		";
	}
	print "<a href=\"profiles/$newentry.htm\">$newentry</a> <br>";
     } 
   }

  closedir($d);
 } 
}

getNames("profiles/"); 
?>

</td>
</tr>
</table>
</BODY>
</HTML>
