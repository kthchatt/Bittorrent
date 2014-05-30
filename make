clear
echo "       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        "
echo "       ^ Cookie-Crumb BitTorrent v1.0b ^        "
echo "       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        "
echo;echo
echo "                    |__|                         "      
echo "           ~||  «&&&&||&&&»                      "      
echo "         ~&&|| «&&&&&||&&&&&»                    "            
echo "       ~&&&&|| «&&&&&||&&&&&»                    "     
echo "      ~&&&&&||  «&&&&||&&&&»                     " 
echo "       ~&&&&||       ||          «|              "
echo "  \-----æ--------æ-----æ-----æ----//             "
echo "   \-- o ---- o ---- o ---- o--t-//f             "
echo ",,,,\___________________________//,,,,,,,,,,,,,,,"
echo;echo
echo -n "Setting sails... "
gcc -g -Wall netstat.c rss2.c MOTD.c bitfield.c swarm.c peerwire.c urlparse.c protocol_tcp.c protocol_udp.c announce.c scrape.c tracker.c protocol_meta.h bencodning.c createfile.c init_torrent.c SearchPice.c writepiece.c GUI.c -pthread -o ccBitTorrent `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` -lcrypto -lssl
echo -n " Done."
./ccBitTorrent
