<HTML>
<BODY bgcolor="black" text="white" background="back/homeback.jpg" bgproperties="fixed">
<HEAD>
<META HTTP-EQUIV="Refresh" CONTENT="45; URL=webwho.php">
</HEAD>
<br><br>
<font face="Courier" size="3">
<b>
<PRE>
<?
 if(!isSet($file))
  {
    $file = "WEBWHO";
  }
  $fcontents = file($file);
  for ($i=0;$i<=sizeof($fcontents);$i++)
  {
    print($fcontents[$i]);
  }
?>
</PRE>
</b>
</font>
</BODY>
</HTML>
