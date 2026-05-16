#include <stdio.h>

int main() {
    int bucket_size, output_rate, n, i;
    printf("Enter bucket size: ");
    scanf("%d", &bucket_size);
    printf("Enter output rate: ");
    scanf("%d", &output_rate);
    printf("Enter number of packets: ");
    scanf("%d", &n);

    int packets[n];
    for(i = 0; i < n; i++) {
        printf("Enter size of packet %d: ", i+1);
        scanf("%d", &packets[i]);
    }

    int sec = 0;
    int remain = 0; // current content in bucket
    int pkt_index = 0;

    printf("\nTIME\tRECIEVED\tSENT\tDROPPED\tREMAINING\n");

    while(pkt_index < n || remain > 0) {
        int received = 0, dropped = 0, sent = 0;

        // Receive packet if available
        if(pkt_index < n) {
            received = packets[pkt_index];
            if(remain + received <= bucket_size) {
                remain += received;
            } else {
                dropped = (remain + received) - bucket_size;
                remain = bucket_size;
            }
            pkt_index++;
        }

        // Send output_rate from bucket
        if(remain >= output_rate) {
            sent = output_rate;
            remain -= output_rate;
        } else {
            sent = remain;
            remain = 0;
        }

        sec++;
        printf("%d\t%d\t\t%d\t%d\t%d\n", sec, received, sent, dropped, remain);
    }

    return 0;
}
