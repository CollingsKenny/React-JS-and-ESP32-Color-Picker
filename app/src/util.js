export async function postLed(hex) {
  await fetch(`/api/led`, {
    method: "POST",
    body: parseInt(hex.substr(1), 16),
  });
}
