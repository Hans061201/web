<?php
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "conexionprueba";

// Crear conexión
$conn = new mysqli($servername, $username, $password, $dbname);

// Verificar conexión
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT id, nombre, apellido, hora FROM asistencia";
$result = $conn->query($sql);

$asistencias = array();

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $asistencias[] = $row;
    }
}

echo json_encode($asistencias);

$conn->close();
?>
