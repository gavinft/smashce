#include "controller.h"
#include <stdlib.h>
#include <string.h>

void xbc_Init(xbc_controller_t* controller, usb_device_t device) {
    size_t descriptor_length = usb_GetConfigurationDescriptorTotalLength(device, 0);

    usb_configuration_descriptor_t* conf_descriptor = malloc(descriptor_length); // could statically allocate 153 bytes

    usb_GetConfigurationDescriptor(device, 0, conf_descriptor, descriptor_length, NULL);
    usb_SetConfiguration(device, conf_descriptor, 49 /* descriptor_length (like i guess it doesnt have to be this big)*/);

    free(conf_descriptor);

    controller->control_endpoint = usb_GetDeviceEndpoint(device, 0x81);
    controller->rumble_led_endpoint = usb_GetDeviceEndpoint(device, 0x01);

}


static usb_error_t xbc_ScanCallback(usb_endpoint_t endpoint, usb_transfer_status_t status, size_t transferred, void *data) {

    *(bool*)data = false;

    return USB_SUCCESS;
}

void xbc_Scan(xbc_controller_t* controller) {

    if (controller->transfer_active) return;

    usb_error_t err = usb_ScheduleTransfer(controller->control_endpoint, &controller->control_data, sizeof(xbc_control_data_t), xbc_ScanCallback, &controller->transfer_active);
    if (err == USB_SUCCESS)
        controller->transfer_active = true;
    
}

void xbc_SetRumble(xbc_controller_t* controller, uint8_t light_rumble, uint8_t heavy_rumble) {
    xbc_rumble_led_data_t data = { 0 };
    data.size = XBC_RUMBLE_PACKET_SIZE;
    data.mode = XBC_SET_RUMBLE;

    data.heavy_rumble = heavy_rumble;
    data.light_rumble = light_rumble;
    
    usb_Transfer(controller->rumble_led_endpoint, &data, data.size, 0, NULL);
}

void xbc_SetLED(xbc_controller_t* controller, xbc_led_mode led_mode) {
    xbc_rumble_led_data_t data;
    data.size = XBC_LED_PACKET_SIZE;
    data.mode = XBC_SET_LED;

    data.led_mode = led_mode;

    usb_Transfer(controller->rumble_led_endpoint, &data, data.size, 0, NULL);
}