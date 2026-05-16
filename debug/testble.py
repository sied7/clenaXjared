import asyncio
from bleak import BleakClient

ADDRESS = "68:67:25:EC:83:4A"  # <-- replace with your ESP32 BLE MAC
CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

async def main():
    print("Connecting to ESP32 BLE device...")
    async with BleakClient(ADDRESS) as client:
        print("Connected. Press Ctrl+C to exit.")

        while True:
            try:
                # Ask user for a number
                user_input = input("Enter a number to send: ")

                # Validate input
                try:
                    value = int(user_input)
                except ValueError:
                    print("Please enter a valid integer.")
                    continue

                # Convert to 4-byte little-endian integer
                data = value.to_bytes(4, byteorder="little", signed=False)

                # Send to ESP32
                await client.write_gatt_char(CHAR_UUID, data)
                print(f"Sent {value} to ESP32")

            except KeyboardInterrupt:
                print("\nExiting...")
                break

asyncio.run(main())
