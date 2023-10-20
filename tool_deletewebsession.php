<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"delete from mysession where ip like 'WEB.%'");
oci_execute($query);
oci_free_statement($query);

oci_close($conn);

?>
