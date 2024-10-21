void setup() {
  Serial.begin(115200);
  delay(1000);

  // Obtener el Chip ID (MAC address completo)
  uint64_t chipid = ESP.getEfuseMac();

  // Mostrar el Chip ID en hexadecimal, ajustando a 16 caracteres con ceros al principio
  Serial.print("Chip ID: ");
  Serial.printf("%016llX\n", chipid);  // Mostrar el valor completo como 16 caracteres
}

void loop() {
  // No se necesita hacer nada en el loop para este ejemplo
}
