
#include <device.h>
#include <init.h>

#include <math.h>

#include <settings/settings.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <zmk/keys.h>

static struct bt_conn *auth_passkey_entry_conn;
static u8_t passkey_entries[6] = {0, 0, 0, 0, 0, 0};
static u8_t passkey_digit = 0;

static void connected(struct bt_conn *conn, u8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        printk("Failed to connect to %s (%u)\n", addr, err);
        return;
    }

    printk("Connected %s\n", addr);

    if (bt_conn_set_security(conn, BT_SECURITY_L2))
    {
        printk("Failed to set security\n");
    }
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Disconnected from %s (reason 0x%02x)\n", addr, reason);
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err)
    {
        printk("Security changed: %s level %u\n", addr, level);
    }
    else
    {
        printk("Security failed: %s level %u err %d\n", addr, level,
               err);
    }
}

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = security_changed,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Passkey for %s: %06u\n", addr, passkey);
}

#ifdef CONFIG_ZMK_BLE_PASSKEY_ENTRY

static void auth_passkey_entry(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Passkey entry requested for %s\n", addr);
    auth_passkey_entry_conn = bt_conn_ref(conn);
}

#endif

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (auth_passkey_entry_conn)
    {
        bt_conn_unref(auth_passkey_entry_conn);
        auth_passkey_entry_conn = NULL;
    }

    passkey_digit = 0;

    printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb zmk_ble_auth_cb_display = {
// .passkey_display = auth_passkey_display,

#ifdef CONFIG_ZMK_BLE_PASSKEY_ENTRY
    .passkey_entry = auth_passkey_entry,
#endif
    .cancel = auth_cancel,
};

static const struct bt_data zmk_ble_ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  0x12, 0x18,  /* HID Service */
                  0x0f, 0x18), /* Battery Service */
};

static void zmk_ble_ready(int err)
{
    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, zmk_ble_ad, ARRAY_SIZE(zmk_ble_ad), NULL, 0);
    if (err)
    {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }
}

static int zmk_ble_init(struct device *_arg)
{
    if (IS_ENABLED(CONFIG_SETTINGS))
    {
        settings_load();
    }
    int err = bt_enable(zmk_ble_ready);

    if (err)
    {
        printk("BLUETOOTH FAILED");
        return err;
    }

    bt_conn_cb_register(&conn_callbacks);
    bt_conn_auth_cb_register(&zmk_ble_auth_cb_display);

    return 0;
}

bool zmk_ble_handle_key_user(struct zmk_key_event *key_event)
{
    zmk_key key = key_event->key;

    if (!auth_passkey_entry_conn)
    {
        return true;
    }

    if (key < NUM_1 || key > NUM_0)
    {
        return true;
    }

    u32_t val = (key == NUM_0) ? 0 : (key - NUM_1 + 1);

    passkey_entries[passkey_digit++] = val;

    if (passkey_digit == 6)
    {
        u32_t passkey = 0;
        for (int i = 5; i >= 0; i--)
        {
            passkey = (passkey * 10) + val;
        }
        bt_conn_auth_passkey_entry(auth_passkey_entry_conn, passkey);
        bt_conn_unref(auth_passkey_entry_conn);
        auth_passkey_entry_conn = NULL;
    }

    return false;
}

SYS_INIT(zmk_ble_init,
        APPLICATION,
        CONFIG_ZMK_BLE_INIT_PRIORITY);
