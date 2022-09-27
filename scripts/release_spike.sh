#!/bin/bash

usage() {
	echo "Usage: $0 [-l LINK_NAME] <-b BUILD_DIR> <-v OS_VERSION>"
	echo
	exit 1
}

if [ x"$CI_COMMIT_REF_NAME" = x"master" ]; then
	export LINK=latest
else
	export LINK=`echo $CI_COMMIT_REF_NAME | sed 's#/#-#g'`
fi

BUILD_DIR=""
OS_VERSION=""
while getopts "l:b:v:" arg; do
    case $arg in
    l)
        LINK="$OPTARG"
        ;;
    b)
        BUILD_DIR="$OPTARG"
        ;;
    v)
        OS_VERSION="$OPTARG"
        ;;
    ?)
        echo "Unkonw argument"
        usage
        ;;
    esac
done

if [ -z "$BUILD_DIR" -o -z "$OS_VERSION" ]; then
    usage
fi

export VER=`echo $CI_COMMIT_REF_NAME | sed 's#/#-#g'`-`date +%Y%m%d`.$CI_PIPELINE_ID
cd $BUILD_DIR
make install
mkdir -p archive
`dirname $0`/package.sh `pwd`/install archive/spike-installer-$VER.sh
ln -sf archive/spike-installer-$VER.sh $LINK

command -v ssh-agent >/dev/null || ( apt-get update -y && apt-get install openssh-client -y )
eval $(ssh-agent -s)
echo "$DEPLOY_PRIVATE_KEY" | tr -d '\r' | ssh-add -
mkdir -p ~/.ssh && chmod 700 ~/.ssh
echo "$DEPLOY_KNOWN_HOSTS" >> ~/.ssh/known_hosts
chmod 644 ~/.ssh/known_hosts

ssh $DEPLOY_USER@$DEPLOY_SERVER "if [ ! -d ~/files/simulator/spike/$OS_VERSION ]; then mkdir -p ~/files/simulator/spike/$OS_VERSION; fi"
tar -c archive $LINK | ssh $DEPLOY_USER@$DEPLOY_SERVER "tar -xvC ~/files/simulator/spike/$OS_VERSION"
