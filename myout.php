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

function show1($table,$par,$title,$istat,$sovra,$conn,$privacy){
  global $comune,$unione,$regione;
  $query=oci_parse($conn,"select $par from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  if($aux<3 & $privacy)$aux="*";
  oci_free_statement($query);
  $comune.="    \"$table\":$aux,\n";
  $query=oci_parse($conn,"select $par from $table where istat='$sovra'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  if($aux<3 & $privacy)$aux="*";
  oci_free_statement($query);
  $unione.="    \"$table\":$aux,\n";
  $query=oci_parse($conn,"select $par from $table where istat='00008'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  if($aux<3 & $privacy)$aux="*";
  oci_free_statement($query);
  $regione.="    \"$table\":$aux,\n";
}

show1("uiftth","uiftth","FTTH bianche",$istat,$sovra,$conn,0);
show10("areeaai","areeaai","AAI Aree",$istat,$sovra,$conn,0,$tt);
show10("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$conn,0,$tt);
show10("scuole","scuole","Scuole 1G",$istat,$sovra,$conn,0,$tt);
show10("pal","pal","PAL rete",$istat,$sovra,$conn,0,$tt);
show10("man","man","MAN rete",$istat,$sovra,$conn,0,$tt);
show10("apwifi","apwifi","Punti WiFi",$istat,$sovra,$conn,0,$tt);
show10("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$conn,1,$tt);
show10("attivifse","attivi","Attivi FSE",$istat,$sovra,$conn,1,$tt);
show10("accessifse","accessi","Accessi FSE",$istat,$sovra,$conn,1,$tt);
show10("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$conn,1,$tt);
show10("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$conn,1,$tt);
show10("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$conn,1,$tt);
show10("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$conn,0,$tt);

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

function show10($table,$par,$title,$istat,$sovra,$conn,$privacy,$tt){
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
