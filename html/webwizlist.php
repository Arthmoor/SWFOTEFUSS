<HTML>
<BODY bgcolor="black" text="white" background="back/homeback.jpg" bgproperties="fixed">
<font face="Courier" size="3">
<b>
<PRE>
<?
 if(!isSet($file))
  {
    $file = "WEBWIZLIST";
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
