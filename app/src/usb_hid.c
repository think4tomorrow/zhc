
#include <device.h>
#include <init.h>

#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>
#include <dt-bindings/zmk/keys.h>

#include <zmk/hid.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static enum usb_dc_status_code usb_status;

static struct device *hid_dev;

int zmk_usb_hid_send_report(const u8_t *report, size_t len)
{
	if (usb_status == USB_DC_SUSPEND)
	{
		return usb_wakeup_request();
	}

	return hid_int_ep_write(hid_dev, report, len, NULL);
}

void usb_hid_status_cb(enum usb_dc_status_code status, const u8_t *params)
{
	usb_status = status;
};

static int zmk_usb_hid_init(struct device *_arg)
{
	int usb_enable_ret;

	hid_dev = device_get_binding("HID_0");
	if (hid_dev == NULL)
	{
		LOG_ERR("Unable to locate HID device");
		return -EINVAL;
	}

	usb_hid_register_device(hid_dev,
							zmk_hid_report_desc, sizeof(zmk_hid_report_desc),
							NULL);

	usb_hid_init(hid_dev);

	usb_enable_ret = usb_enable(usb_hid_status_cb);

	if (usb_enable_ret != 0)
	{
		LOG_ERR("Unable to enable USB");
		return -EINVAL;
	}

	return 0;
}

SYS_INIT(zmk_usb_hid_init,
         APPLICATION,
		 CONFIG_ZMK_USB_INIT_PRIORITY);
