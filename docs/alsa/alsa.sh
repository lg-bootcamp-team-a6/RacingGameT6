# NOTICE: /nfsroot 하위에 위치해야 함

ALSA_CONFIG_PATH=/mnt/nfs/alsa-lib/share/alsa/alsa.conf
export ALSA_CONFIG_PATH
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/nfs/alsa-lib/lib:/mnt/nfs/alsa-lib/lib/alsa-lib:/mnt/nfs/alsa-lib/lib/alsa-lib/smixer
export LD_LIBRARY_PATH

# /mnt/nfs 에서 source ./alsa.sh 실행