#define LOG_TAG "bdaddr_read"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>

#define PERSIST_BT_NV_PATH "/persist/bluetooth/.bt_nv.bin"
#define DATA_BDADDR_PATH "/data/misc/bluetooth/bdaddr"

/*
 * Read MAC address from persist partition.
 *
 * The file is in this format:
 * 01 01 06 ff ff 00 2d 65 94
 * for 94-65-2d-00-ff-ff
 */
static int bt_addr_read_from_persist(uint8_t *addr_out) {
    int ret = 0;
    if (addr_out == NULL) {
        ret = -EINVAL;
        goto exit;
    }

    FILE *p_nv_bin = fopen(PERSIST_BT_NV_PATH, "rb");
    if (p_nv_bin == NULL) {
        ret = -errno;
        perror("Could not open BT NV file from persist");
        goto exit;
    }

    if (fseek(p_nv_bin, 3L, SEEK_SET) < 0) {
        ret = -errno;
        perror("Could not seek to position 3");
        goto close_exit;
    }

    if (fread(addr_out, 6, 1, p_nv_bin) < 1) {
        ret = -errno;
        perror("Could not read all 6 bytes of MAC address");
        goto close_exit;
    }

close_exit:
    fclose(p_nv_bin);
exit:
    return ret;
}

/*
 * Write address to data partition
 * so that AOSP bluetooth HAL can read it
 */
static int bt_addr_print_to_data(const uint8_t *addr) {
    int ret = 0, print_len = 0;
    char addr_str_buf[19] = {0}; /* 00-00-00-00-00-00\n\000 */

    if (addr == NULL) {
        ret = -EINVAL;
        goto exit;
    }

    FILE *p_data_txt = fopen(DATA_BDADDR_PATH, "w");
    if (p_data_txt == NULL) {
        ret = -errno;
        perror("Could not open bdaddr file for writing");
        goto exit;
    }

    print_len = snprintf(addr_str_buf, sizeof(addr_str_buf),
                         "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n", addr[0],
                         addr[1], addr[2], addr[3], addr[4], addr[5]);

    if (fwrite(addr_str_buf, print_len, 1, p_data_txt) < 1) {
        ret = -errno;
        perror("Could not write to bdaddr");
    }

    fclose(p_data_txt);
exit:
    return ret;
}

/*
 * Swap byte order
 */
static void bt_addr_invert(uint8_t *addr) {
    uint8_t tmp;

    for (int pos = 0; pos < 3; pos++) {
        tmp = addr[pos];
        addr[pos] = addr[5 - pos];
        addr[5 - pos] = tmp;
    }
}

int main(int argc, char *argv[]) {
    uint8_t addr[6] = {0};
    int ret = EXIT_SUCCESS;

    if (bt_addr_read_from_persist(addr) < 0) {
        ret = EXIT_FAILURE;
        goto exit;
    }

    bt_addr_invert(addr);
    ALOGV("Got BD address from persist: "
          "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
          addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    if (bt_addr_print_to_data(addr) < 0) {
        ret = EXIT_FAILURE;
        goto exit;
    }

exit:
    return ret;
}
