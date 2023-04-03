#ifndeF K_VIRTIO_H
#define K_VIRTIO_H

enum K_VIRTIO_STATUS
{
    K_VIRTIO_STATUS_ACK             = 1,
    K_VIRTIO_STATUS_DRIVER          = 1 << 1,
    K_VIRTIO_STATUS_DRV_OK          = 1 << 2,
    K_VIRTIO_STATUS_FEAT_OK         = 1 << 3,
    K_VIRTIO_STATUS_RESET           = 1 << 6,
    K_VIRTIO_STATUS_FAILED          = 1 << 7,
};

#endif