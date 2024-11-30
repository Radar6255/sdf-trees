sudo docker build -t app-image:latest . 
sudo docker run -v $PWD:/opt/mount --rm -ti app-image:latest bash -c "cp /demo-x86_64.AppImage /opt/mount/."
