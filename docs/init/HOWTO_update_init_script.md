# 환경설정) 부팅시 자동 실행
 (리눅스) ~/work/buildroot-2024.02.1/rootfs/etc/init.d/S50local 아래에 추가하고 싶은 스크립트 수정
참고 : 부팅시 실행되므로 웬만한 커맨드는 & 붙여 백그라운드에서 실행
(리눅스) ~/work/buildroot-2024.02.1에서 ./mkext2.sh로 RFS 굽굽 && cp ext2img.gz /nfsroot
 (타겟) Uboot 진입 후 run RFS
재부팅하면 부팅 제일 마지막 시퀀스에서 스크립트 실행됨

### 1) Virtual Machine (Ubuntu) 환경서 실행
1. `vim ~/work/buildroot-2024.02.1/rootfs/etc/init.d/S50local`
2. 아래와 같이 내용 수정
    ```
    # Wait for ethernet device to link
    for count in 1 2 3 4 5
    do
            link=`cat /sys/class/net/eth0/carrier`
            if [ "$link" = "1" ]; then break; fi
            echo "Wait for ethernet device to link - $count"
            sleep 1
    done

    mknod /dev/mydev c 120 0
    mount -t nfs -o nolock 192.168.10.2:/nfsroot /mnt/nfs

    # Racing2D Setting
    export QT_QPA_PLATFORM=linuxfb

    cd /mnt/nfs
    insmod devtest.ko

    ./Racing2D &
    ```

[ ] TODO: ++ ALSA 드라이버 추가시 `source /mnt/nfs/alsa.sh` 추가 필요
3. `sudo` > 비밀번호 입력
4. `cd ~/work/buildroot-2024.02.1 && sudo ./mkext2.sh && sudo cp ext2img.gz /nfsroot`

### 2) Target (보드) 에서 실행
1. Uboot 진입
2. `run RFS`
3. 재부팅