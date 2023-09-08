#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <json-glib/json-glib.h>
#include <stdio.h>
#include <stddef.h>
#include <rtc/rtc.h>
struct sockaddr_in addr;
int socket_desc;
int pc;
char * generateSdpJson(char * sdp) {
  JsonBuilder *builder = json_builder_new ();
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "type");
  json_builder_add_string_value (builder, "offer");
  json_builder_set_member_name (builder, "sdp");
  json_builder_add_string_value (builder, sdp);
  json_builder_end_object (builder);
  
  JsonGenerator *gen = json_generator_new ();
  JsonNode * root = json_builder_get_root (builder);
  json_generator_set_root (gen, root);
  gchar *str = json_generator_to_data (gen, NULL);
  json_node_free (root);
  g_object_unref (gen);
  g_object_unref (builder);
  
  return str;
}

char * extractSdpFromJson(char * json) {
  JsonParser *parser = json_parser_new ();
  json_parser_load_from_data (parser, json, -1, NULL);
  JsonReader *reader = json_reader_new (json_parser_get_root (parser));
  json_reader_read_member (reader, "sdp");
  const char * val = json_reader_get_string_value (reader);
  char *url = strdup(json_reader_get_string_value (reader));
  
  json_reader_end_member (reader); 
  g_object_unref (reader);
  g_object_unref (parser);
  return url;
}

char * rtcGatheringStateText(rtcGatheringState state) {
  switch (state) {
  case RTC_GATHERING_NEW:
    return "NEW";
  case RTC_GATHERING_INPROGRESS:
    return "INPROGRESS";
  case RTC_GATHERING_COMPLETE:
    return "COMPLETE";
  }
};

void myMessageCallback(int id, const char *message, int size, void *user_ptr) {
  /* printf("got message\n"); */
  sendto(socket_desc, message, size, 0, (struct sockaddr*)&addr, sizeof(addr));
}

void myGatheringStateCallback(int pc, rtcGatheringState state, void *user_ptr) {
  printf("Gathering State: %s\n", rtcGatheringStateText(state));
  if (state == RTC_GATHERING_COMPLETE) {
    char description [1000];
    if (rtcGetLocalDescription(pc, description, 1000)<0) {
      perror("Couldn't get local description");
      return;
    }
    char * sdpJson = generateSdpJson(description);
    printf("\n%s\n", sdpJson);
    free(sdpJson);
  }
}

int main() {
  rtcInitLogger(RTC_LOG_VERBOSE, NULL);
  rtcConfiguration conf = {0};
  pc = rtcCreatePeerConnection(&conf);
  rtcSetGatheringStateChangeCallback(pc, myGatheringStateCallback);

  socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(5000);
  
  int tr = rtcAddTrack(pc, "video 52313 UDP/TLS/RTP/SAVPF 96\n"
	      "a=mid:video\n"
	      "a=recvonly\n"
	      "b=AS:3000\n"
	      "a=rtpmap:96 H264/90000\n"
	      "a=rtcp-fb:96 nack\n"
	      "a=rtcp-fb:96 nack pli\n"
	      "a=rtcp-fb:96 goog-rem\n"
	      "a=fmtp:96 profile-level-id=42e01f;packetization-mode=1;\
level-asymmetry-allowed=1\n"
	      );
  rtcSetMessageCallback(tr, myMessageCallback);
  rtcSetLocalDescription(pc, NULL);
  char remote_description_buf[10000];
  fgets(remote_description_buf, 10000, stdin);
  char * remote_description = extractSdpFromJson(remote_description_buf);
  rtcSetRemoteDescription(pc, remote_description, "answer");
  getchar();
  return 0;
}
