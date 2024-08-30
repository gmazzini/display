<?php

include "data.php";
$istat=$_GET["istat"];
if(isset($_GET["tt"]))$tt=(int)$_GET["tt"];
else $tt=(int)(time()/86400);

$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select sovra from idistat where istat='$istat'");
oci_execute($query);
$row=oci_fetch_row($query);
@$sovra=$row[0];
oci_free_statement($query);
$comune="  \"comune\":{\n";
$unione="  \"unione\":{\n";
$regione="  \"regione\":{\n";

$query=oci_parse($conn,"select ente from istatente where istat='$istat'");
oci_execute($query);
$row=oci_fetch_row($query);
@$ente=$row[0];
oci_free_statement($query);
$comune.="    \"ente\":\"$ente\",\n";

$query=oci_parse($conn,"select ente from istatente where istat='$sovra'");
oci_execute($query);
$row=oci_fetch_row($query);
@$ente=$row[0];
oci_free_statement($query);
$unione.="    \"ente\":\"$ente\",\n";

show10("uiftth","uiftth",$istat,$sovra,$conn,0,$tt);
show10("uifwa","uifwa",$istat,$sovra,$conn,0,$tt);
show10("areeaai","areeaai",$istat,$sovra,$conn,0,$tt);
show10("aziendeaai","aziendeaai",$istat,$sovra,$conn,0,$tt);
show10("scuole","scuole",$istat,$sovra,$conn,0,$tt);
show10("pal","pal",$istat,$sovra,$conn,0,$tt);
show10("man","man",$istat,$sovra,$conn,0,$tt);
show10("apwifi","apwifi",$istat,$sovra,$conn,0,$tt);
show10("userwifi","userwifi",$istat,$sovra,$conn,1,$tt);
show10("attivifse","attivi",$istat,$sovra,$conn,1,$tt);
show10("accessifse","accessi",$istat,$sovra,$conn,1,$tt);
show10("scaricatifse","scaricati",$istat,$sovra,$conn,1,$tt);
show10("attivazionilepidaid","attivazioni",$istat,$sovra,$conn,1,$tt);
show10("accessilepidaid","accessi",$istat,$sovra,$conn,1,$tt);
show10("sportellilepidaid","sportelli",$istat,$sovra,$conn,0,$tt);
show10("sensori","sensori",$istat,$sovra,$conn,1,$tt);

$comune.="    \"istat\":\"$istat\"\n  },\n";
$regione.="    \"istat\":\"00008\"\n  }";
if($sovra=="")$regione.="\n";
else {
  $regione.=",\n";
  $unione.="    \"istat\":\"$sovra\"\n  }\n";
}
echo "{\n";
echo "  \"epoch\":".time().",\n";
echo "  \"tt\":$tt,\n";
echo $comune;
echo $regione;
if($sovra<>"")echo $unione;
echo "}\n";

oci_close($conn);

function show10($table,$par,$istat,$sovra,$conn,$privacy,$tt){
  global $comune,$unione,$regione;
  $query=oci_parse($conn,"select $par from $table where istat='$istat' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  if($aux<3 & $privacy)$aux="*";
  oci_free_statement($query);
  $comune.="    \"$table\":$aux,\n";
  $query=oci_parse($conn,"select $par from $table where istat='$sovra' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  if($aux<3 & $privacy)$aux="*";
  oci_free_statement($query);
  $unione.="    \"$table\":$aux,\n";
  $query=oci_parse($conn,"select $par from $table where istat='00008' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  if($aux<3 & $privacy)$aux="*";
  oci_free_statement($query);
  $regione.="    \"$table\":$aux,\n";
}

?>
