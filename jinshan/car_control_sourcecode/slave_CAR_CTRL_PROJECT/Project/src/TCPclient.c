/**
  ******************************************************************************
  * @file    tcp_echoclient.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   tcp echoclient application using LwIP RAW API
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "stm32f10x.h"
#include "memp.h"
#include "netconf.h"
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "TCPclient.h"




/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

struct tcp_client_table	  tcp_client_list[TCP_CLIENT_MAX_NUM];
u8 heart_dely = 0;
u8 TCP_send_dely = 0;
u16 WCS_ACK_timer = 0;
extern u8 msg_send_en;

/* ECHO protocol states */
enum echoclient_states
{
    ES_NOT_CONNECTED = 0,
    ES_CONNECTED,
    ES_RECEIVED,
    ES_CLOSING,
};


/* structure to be passed as argument to the tcp callbacks */
struct echoclient
{
    enum echoclient_states state; /* connection status */
    struct tcp_pcb *pcb;          /* pointer on the current tcp_pcb */
    struct pbuf *p_tx;            /* pointer on pbuf to be transmitted */
};

void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void get_ip_para_process(u8 tcp_offset);
#ifndef     USE_UDP

#if LWIP_TCP

/* Private function prototypes -----------------------------------------------*/
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient *es);
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient *es);
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

/* Private functions ---------------------------------------------------------*/

void creat_tcp_connect(u8 tcp_offset)
{
#if LWIP_DHCP
    if( DHCP_state == DHCP_ADDRESS_ASSIGNED )
    {
        /* create new tcp pcb */
        if( tcp_client_list[tcp_offset].echoclient_pcb == NULL)
        {
            tcp_client_list[tcp_offset].echoclient_pcb = tcp_new();
            tcp_client_list[tcp_offset].echoclient_pcb->local_port = 0;
            tcp_client_list[tcp_offset].connect_is_ok = 1;
            tcp_client_list[tcp_offset].connect_dely = 0;
            tcp_client_list[tcp_offset].tcp_send_en = 0;
            tcp_client_list[tcp_offset].tcp_send_len = 0;
        }
        tcp_client_list[tcp_offset].tcp_client_statue = CLIENT_DIS_CONNECT;
    }
    else
#else
    {
        /* create new tcp pcb */
        if( [tcp_offset].echoclient_pcb == NULL)
        {
            tcp_client_list[tcp_offset].echoclient_pcb = tcp_new();
            tcp_client_list[tcp_offset].echoclient_pcb->local_port = 0;
            tcp_client_list[tcp_offset].connect_is_ok = 1;
            tcp_client_list[tcp_offset].connect_dely = 0;
            tcp_client_list[tcp_offset].tcp_send_en = 0;
            tcp_client_list[tcp_offset].tcp_send_len = 0;
        }
        tcp_client_list[tcp_offset].tcp_client_statue = CLIENT_DIS_CONNECT;
    }
#endif
    }

/**
  * @brief  Connects to the TCP echo server
  * @param  None
  * @retval None
  */
void tcp_echoclient_connect(u8 tcp_offset)
{
    struct ip_addr DestIPaddr;
    //struct netif *netif;

#if LWIP_DHCP
    if(DHCP_state != DHCP_ADDRESS_ASSIGNED)
    {
        return;
    }
#endif
    if(tcp_client_list[tcp_offset].connect_is_ok == 0)
    {
        creat_tcp_connect(tcp_offset);
    }
    if (tcp_client_list[tcp_offset].echoclient_pcb != NULL)
    {
        get_ip_para_process(tcp_offset);
        IP4_ADDR( &DestIPaddr, tcp_client_list[tcp_offset].dest_ip[0], tcp_client_list[tcp_offset].dest_ip[1], tcp_client_list[tcp_offset].dest_ip[2], tcp_client_list[tcp_offset].dest_ip[3] );

        tcp_connect(tcp_client_list[tcp_offset].echoclient_pcb, &DestIPaddr, tcp_client_list[tcp_offset].dest_port, tcp_echoclient_connected);
        tcp_client_list[tcp_offset].connect_dely = 3;
    }
}

/**
  * @brief Function called when TCP connection established
  * @param tpcb: pointer on the connection contol block
  * @param err: when connection correctly established err should be ERR_OK
  * @retval err_t: returned error
  */
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    struct echoclient *es = NULL;
    u8 i, j, tmp;

    if (err == ERR_OK)
    {
        /* allocate structure es to maintain tcp connection informations */
        es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));

        if (es != NULL)
        {
            es->state = ES_CONNECTED;
            es->pcb = tpcb;

            /* allocate pbuf */
            es->p_tx = pbuf_alloc(PBUF_TRANSPORT, 0 , PBUF_POOL);

            if (es->p_tx)
            {
                /* copy data to pbuf */
                //pbuf_take(es->p_tx, (char*)data, strlen((char*)data));

                /* pass newly allocated es structure as argument to tpcb */
                tcp_arg(tpcb, es);

                /* initialize LwIP tcp_recv callback function */
                tcp_recv(tpcb, tcp_echoclient_recv);

                /* initialize LwIP tcp_sent callback function */
                tcp_sent(tpcb, tcp_echoclient_sent);

                /* initialize LwIP tcp_poll callback function */
                tcp_poll(tpcb, tcp_echoclient_poll, 1);
                /*USART_SendData(DEBUG_USART, '#');
                while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET);  */
                /* send data */
                //tcp_echoclient_send(tpcb,es);
                for(i = 0; i < TCP_CLIENT_MAX_NUM; i++)
                {
                    if( tpcb->remote_port == tcp_client_list[i].dest_port )
                    {
                        for(j = 0; j < 4; j++ )
                        {
                            tmp = tpcb->remote_ip.addr  >> ( 8 * j );
                            if( tcp_client_list[i].dest_ip[j] != tmp )
                            {
                                break;
                            }
                        }
                        if(j == 4)
                        {
                            tcp_client_list[i].tcp_client_statue = CLIENT_CONNECT_OK;
                            tcp_client_list[i].connect_dely = 0;
                            if( i == 0 )
                            {
                                AddSendMsgToQueue(SEND_MSG_CAR2WCS_CMD_TYPE);
                                TCP_send_dely = 200;
                                recive_wcs_count = 0;
                            }
                        }
                    }
                }

                return ERR_OK;
            }
        }
        else
        {
            /* close connection */
            tcp_echoclient_connection_close(tpcb, es);

            /* return memory allocation error */
            return ERR_MEM;
        }
    }
    else
    {
        /* close connection */
        tcp_echoclient_connection_close(tpcb, es);
    }
    return err;
}

/**
  * @brief tcp_receiv callback
  * @param arg: argument to be passed to receive callback
  * @param tpcb: tcp connection control block
  * @param err: receive error code
  * @retval err_t: retuned error
  */
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct echoclient *es;
    err_t ret_err;
    u8_t tmp;
    u16_t i, j, m;
    u8_t *buf;

    LWIP_ASSERT("arg != NULL", arg != NULL);

    es = (struct echoclient *)arg;

    /* if we receive an empty tcp frame from server => close connection */
    if (p == NULL)
    {
        /* remote host closed connection */
        es->state = ES_CLOSING;
        if(es->p_tx == NULL)
        {
            /* we're done sending, close connection */
            tcp_echoclient_connection_close(tpcb, es);
        }
        else
        {
            /* send remaining data*/
            tcp_echoclient_send(tpcb, es);
        }
        ret_err = ERR_OK;
    }
    /* else : a non empty frame was received from echo server but for some reason err != ERR_OK */
    else if(err != ERR_OK)
    {
        /* free received pbuf*/
        if (p != NULL)
        {
            pbuf_free(p);
        }
        ret_err = err;
    }
    else if(es->state == ES_CONNECTED)
    {
        for(i = 0; i < TCP_CLIENT_MAX_NUM; i++)
        {
            if( tpcb->remote_port == tcp_client_list[i].dest_port )
            {
                for(j = 0; j < 4; j++ )
                {
                    tmp = tpcb->remote_ip.addr  >> ( 8 * j );
                    if( tcp_client_list[i].dest_ip[j] != tmp )
                    {
                        break;
                    }
                }
                if( ( tcp_client_list[i].tcp_client_statue == CLIENT_CONNECT_OK ) && (j == 4) )
                {
                    if( p->tot_len <= TCP_RECEV_BUFF_SIZE )
                    {
                        buf = ((u8_t *)p->payload);
                        for(m = 0; m < p->tot_len; m++)
                        {
                            tcp_client_list[i].tcp_recev_buf[tcp_client_list[i].tcp_recev_len + m] = *(buf + m);
                        }
                    }
                    tcp_client_list[i].tcp_recev_len += m;
                    tcp_client_list[i].tcp_client_statue = CLIENT_CONNECT_RECV;
                    if( i == 0 )
                    {
                        WCS_ACK_timer = 0;
                    }
                    /*if(i == 0 )
                    {
                        recv_message_from_sever(&(tcp_client_list[i].tcp_recev_buf[0]),&(tcp_client_list[i].tcp_recev_len));
                    } */
                }
            }
        }

        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);

        pbuf_free(p);
        //tcp_echoclient_connection_close(tpcb, es);
        ret_err = ERR_OK;
    }

    /* data received when connection already closed */
    else
    {
        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);

        /* free pbuf and do nothing */
        pbuf_free(p);
        ret_err = ERR_OK;
    }
    return ret_err;
}

/**
  * @brief function used to send data
  * @param  tpcb: tcp control block
  * @param  es: pointer on structure of type echoclient containing info on data
  *             to be sent
  * @retval None
  */
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient *es)
{
    struct pbuf *ptr;
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) &&
            (es->p_tx != NULL) &&
            (es->p_tx->len <= tcp_sndbuf(tpcb)))
    {

        /* get pointer on pbuf from es structure */
        ptr = es->p_tx;

        /* enqueue data for transmission */
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);

        if (wr_err == ERR_OK)
        {
            /* continue with next pbuf in chain (if any) */
            es->p_tx = ptr->next;

            if(es->p_tx != NULL)
            {
                /* increment reference count for es->p */
                pbuf_ref(es->p_tx);
            }

            /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
            pbuf_free(ptr);
        }
        else if(wr_err == ERR_MEM)
        {
            /* we are low on memory, try later, defer to poll */
            es->p_tx = ptr;
        }
        else
        {
            /* other problem ?? */
        }
    }
}

/**
  * @brief  This function implements the tcp_poll callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: tcp connection control block
  * @retval err_t: error code
  */
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct echoclient *es;

    es = (struct echoclient *)arg;
    if (es != NULL)
    {
        if (es->p_tx != NULL)
        {
            /* there is a remaining pbuf (chain) , try to send data */
            tcp_echoclient_send(tpcb, es);
        }
        else
        {
            /* no remaining pbuf (chain)  */
            if(es->state == ES_CLOSING)
            {
                /* close tcp connection */
                tcp_echoclient_connection_close(tpcb, es);
            }
        }
        ret_err = ERR_OK;
    }
    else
    {
        /* nothing to be done */
        tcp_abort(tpcb);
        ret_err = ERR_ABRT;
    }
    return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data)
  * @param  arg: pointer on argument passed to callback
  * @param  tcp_pcb: tcp connection control block
  * @param  len: length of data sent
  * @retval err_t: returned error code
  */
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct echoclient *es;

    LWIP_UNUSED_ARG(len);

    es = (struct echoclient *)arg;

    if(es->p_tx != NULL)
    {
        /* still got pbufs to send */
        tcp_echoclient_send(tpcb, es);
    }

    return ERR_OK;
}

/**
  * @brief This function is used to close the tcp connection with server
  * @param tpcb: tcp connection control block
  * @param es: pointer on echoclient structure
  * @retval None
  */
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient *es )
{
    u8 i, j, tmp;
    /* remove callbacks */
    tcp_recv(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    if (es != NULL)
    {
        mem_free(es);
    }

    /* close tcp connection */
    //tcp_close(tpcb);
    tcp_abort(tpcb);

    for(i = 0; i < TCP_CLIENT_MAX_NUM; i++)
    {
        if( tpcb->remote_port == tcp_client_list[i].dest_port )
        {
            for(j = 0; j < 4; j++ )
            {
                tmp = tpcb->remote_ip.addr  >> ( 8 * j );
                if( tcp_client_list[i].dest_ip[j] != tmp )
                {
                    break;
                }
            }
            if(j == 4)
            {
                tcp_client_list[i].tcp_client_statue = CLIENT_RE_CONNECT;
                tcp_client_list[i].echoclient_pcb = NULL;
                tcp_client_list[i].connect_is_ok = 0;
                tcp_client_list[i].connect_dely = 3;
            }
        }
    }
}
void tcp_client_close_process(u8 client_num)
{
    if( client_num < TCP_CLIENT_MAX_NUM)
    {
        if( tcp_client_list[client_num].tcp_client_statue == CLIENT_CONNECT_OK )
        {
            tcp_echoclient_connection_close(tcp_client_list[client_num].echoclient_pcb, NULL );
        }
    }
}

/*******************************************************************************************

*******************************************************************************************/

/**************************************************************************

**************************************************************************/

/**************************************************************************

**************************************************************************/
void  tcp_client_process(void)
{
    u8 tcp_offset;
    u8 i;
    tcp_offset = 0;
    if(tcp_client_list[tcp_offset].tcp_client_statue == CLIENT_CONNECT_RECV)
    {
        recv_message_from_sever(&(tcp_client_list[tcp_offset].tcp_recev_buf[0]), &(tcp_client_list[tcp_offset].tcp_recev_len));
        tcp_client_list[tcp_offset].tcp_client_statue = CLIENT_CONNECT_OK;
    }
    if( tcp_client_list[tcp_offset].echoclient_pcb->state == CLOSED || tcp_client_list[tcp_offset].echoclient_pcb->state == FIN_WAIT_1
            || tcp_client_list[tcp_offset].echoclient_pcb->state == FIN_WAIT_2 )
    {
        if( tcp_client_list[tcp_offset].connect_dely == 0 )
        {
            tcp_echoclient_connect(tcp_offset);
        }
    }
    else if( tcp_client_list[tcp_offset].echoclient_pcb->state == SYN_SENT )
    {
        if( (tcp_client_list[tcp_offset].echoclient_pcb->nrtx == TCP_SYNMAXRTX ) || ( tcp_client_list[tcp_offset].connect_dely == 0 ) )
        {
            tcp_echoclient_connection_close(tcp_client_list[tcp_offset].echoclient_pcb, NULL );
        }
    }
    else if(tcp_client_list[tcp_offset].connect_is_ok == 0)
    {
        creat_tcp_connect(tcp_offset);
    }
    else if( tcp_client_list[tcp_offset].echoclient_pcb->state == TIME_WAIT )
    {
        tcp_abort(tcp_client_list[tcp_offset].echoclient_pcb);
    }
    else if( tcp_client_list[tcp_offset].echoclient_pcb->state == ESTABLISHED)
    {
        //发送
        if(tcp_client_list[0].tcp_send_en == 1)
        {
            if( tcp_write(tcp_client_list[tcp_offset].echoclient_pcb, &(tcp_client_list[tcp_offset].tcp_send_buf[0]), tcp_client_list[tcp_offset].tcp_send_len, 1) == ERR_OK )
            {
                tcp_client_list[0].tcp_send_en = 0;
                //tcp_output(tcp_client_list[tcp_offset].echoclient_pcb);
            }
            heart_dely = 30;
        }
        /*if( heart_dely == 0 )
        {
            for(i = 0; i< 10; i++)
            {
              tcp_client_list[tcp_offset].tcp_send_buf[i] ='#';
            }
            tcp_write(tcp_client_list[tcp_offset].echoclient_pcb, &(tcp_client_list[tcp_offset].tcp_send_buf[0]), 10, 1);
            heart_dely = 60;
        } */
    }

    tcp_offset = 1;
    if(tcp_client_list[tcp_offset].tcp_client_statue == CLIENT_CONNECT_RECV)
    {
        //recv_message_from_sever(&(tcp_client_list[tcp_offset].tcp_recev_buf[0]),&(tcp_client_list[tcp_offset].tcp_recev_len));
        tcp_client_list[tcp_offset].tcp_client_statue = CLIENT_CONNECT_OK;
    }
    if( tcp_client_list[tcp_offset].echoclient_pcb->state == CLOSED || tcp_client_list[tcp_offset].echoclient_pcb->state == FIN_WAIT_1
            || tcp_client_list[tcp_offset].echoclient_pcb->state == FIN_WAIT_2 )
    {
        if( tcp_client_list[tcp_offset].connect_dely == 0 )
        {
            tcp_echoclient_connect(tcp_offset);
        }
    }
    else if( tcp_client_list[tcp_offset].echoclient_pcb->state == SYN_SENT )
    {
        if( (tcp_client_list[tcp_offset].echoclient_pcb->nrtx == TCP_SYNMAXRTX ) || ( tcp_client_list[tcp_offset].connect_dely == 0 ) )
        {
            tcp_echoclient_connection_close(tcp_client_list[tcp_offset].echoclient_pcb, NULL );
        }
    }
    //else if( ( tcp_client_list[tcp_offset].connect_is_ok == 0 ) && ( tcp_client_list[tcp_offset].connect_dely == 0 ) )
    else if( tcp_client_list[tcp_offset].connect_is_ok == 0 )
    {
        creat_tcp_connect(tcp_offset);
    }
    else if( tcp_client_list[tcp_offset].echoclient_pcb->state == TIME_WAIT )
    {
        tcp_abort(tcp_client_list[tcp_offset].echoclient_pcb);
    }
    else if( tcp_client_list[tcp_offset].echoclient_pcb->state == ESTABLISHED)
    {
        //发送
        /*if(tcp_client_list[0].tcp_send_en == 1)
        {
            if( tcp_write(tcp_client_list[tcp_offset].echoclient_pcb, &(tcp_client_list[tcp_offset].tcp_send_buf[0]), tcp_client_list[tcp_offset].tcp_send_len, 1) == ERR_OK )
            {
            tcp_client_list[0].tcp_send_en = 0;
            }
        }*/
    }
}

#endif /* LWIP_TCP */
#else
/**********************************************************

***********************************************************/
void    creat_udp_connect_process(u8 udp_offset)
{
    tcp_client_list[udp_offset].echoclient_pcb = udp_new();
    tcp_client_list[udp_offset].tcp_client_statue = CLIENT_DIS_CONNECT;
    if (tcp_client_list[udp_offset].echoclient_pcb != NULL)
    {
        tcp_client_list[udp_offset].connect_is_ok = 1;
        tcp_client_list[udp_offset].connect_dely = 0;
        tcp_client_list[udp_offset].tcp_send_en = 0;
        tcp_client_list[udp_offset].tcp_send_len = 0;
    }
}
/***************************************************

**************************************************/
void udp_echoclient_connect(u8 udp_offset)
{
    //struct pbuf *p;
    struct ip_addr DestIPaddr;
    err_t err;

    /*assign destination IP address */
    get_ip_para_process(udp_offset);
    IP4_ADDR( &DestIPaddr, tcp_client_list[udp_offset].dest_ip[0], tcp_client_list[udp_offset].dest_ip[1], tcp_client_list[udp_offset].dest_ip[2], tcp_client_list[udp_offset].dest_ip[3] );
    //IP4_ADDR( &DestIPaddr, 192,168, 10, 110 );
    udp_bind(tcp_client_list[udp_offset].echoclient_pcb, IP_ADDR_ANY, 9000);
    /* configure destination IP address and port */
    err = udp_connect(tcp_client_list[udp_offset].echoclient_pcb, &DestIPaddr, tcp_client_list[udp_offset].dest_port);
    if (err == ERR_OK)
    {
        /* Set a receive callback for the upcb */
        udp_recv(tcp_client_list[udp_offset].echoclient_pcb, udp_receive_callback, NULL);
        tcp_client_list[udp_offset].tcp_client_statue = CLIENT_CONNECT_OK;
        tcp_client_list[udp_offset].connect_dely = 0;
        if( udp_offset == 0 )
        {
            AddSendMsgToQueue(SEND_MSG_CAR2WCS_ONLINE_TYPE);
            TCP_send_dely = 100;
            recive_wcs_count = 0;
        }
    }
    else
    {

    }
}
/**************************************************************************

***************************************************************************/
void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    u8 i, j, tmp, m;
    u8_t *buf;

    for(i = 0; i < TCP_CLIENT_MAX_NUM; i++)
    {
        if( upcb->remote_port == tcp_client_list[i].dest_port )
        {
            for(j = 0; j < 4; j++ )
            {
                tmp = upcb->remote_ip.addr  >> ( 8 * j );
                if( tcp_client_list[i].dest_ip[j] != tmp )
                {
                    break;
                }
            }
            if( ( tcp_client_list[i].tcp_client_statue == CLIENT_CONNECT_OK ) && (j == 4) )
            {
                if( p->tot_len <= TCP_RECEV_BUFF_SIZE )
                {
                    buf = ((u8_t *)p->payload);
                    for(m = 0; m < p->tot_len; m++)
                    {
                        tcp_client_list[i].tcp_recev_buf[tcp_client_list[i].tcp_recev_len + m] = *(buf + m);
                    }
                }
                tcp_client_list[i].tcp_recev_len += m;
                tcp_client_list[i].tcp_client_statue = CLIENT_CONNECT_RECV;
                if( i == 0 )
                {
                    WCS_ACK_timer = 0;
                }
            }
        }
    }
    /* free pbuf */
    pbuf_free(p);

    /* free the UDP connection, so we can accept new clients */
    //udp_remove(upcb);
}
/******************************************************************

******************************************************************/
void  udp_send_process(u8 udp_offset)
{
    struct pbuf *p;

    /* allocate pbuf from pool*/
    p = pbuf_alloc(PBUF_TRANSPORT, tcp_client_list[udp_offset].tcp_send_len, PBUF_POOL);
    if (p != NULL)
    {
        /* copy data to pbuf */
        pbuf_take(p, &(tcp_client_list[udp_offset].tcp_send_buf[0]), tcp_client_list[udp_offset].tcp_send_len);
        /* send udp data */
        udp_send(tcp_client_list[udp_offset].echoclient_pcb, p);

        /* free pbuf */
        pbuf_free(p);
    }
}
/**********************************************************************

***********************************************************************/
void   udp_client_process(void)
{
    if(tcp_client_list[0].tcp_client_statue == CLIENT_CONNECT_RECV)
    {
        recv_message_from_sever(&(tcp_client_list[0].tcp_recev_buf[0]), &(tcp_client_list[0].tcp_recev_len));
        tcp_client_list[0].tcp_client_statue = CLIENT_CONNECT_OK;
    }

    if( tcp_client_list[0].connect_is_ok == 0 )
    {
        creat_udp_connect_process(0);
    }
    else if( tcp_client_list[0].tcp_client_statue == CLIENT_DIS_CONNECT )
    {
        udp_echoclient_connect(0);
    }
    else if(tcp_client_list[0].tcp_client_statue == CLIENT_CONNECT_OK )
    {
        //发送
        if(tcp_client_list[0].tcp_send_en == 1)
        {
            udp_send_process(0);
            tcp_client_list[0].tcp_send_en = 0;
            heart_dely = HEART_DELAY;
        }
    }

    /*if(tcp_client_list[1].tcp_client_statue == CLIENT_CONNECT_RECV)
    {
        recv_message_from_sever(&(tcp_client_list[1].tcp_recev_buf[1]),&(tcp_client_list[1].tcp_recev_len));
        tcp_client_list[1].tcp_client_statue = CLIENT_CONNECT_OK;
    }

    if( tcp_client_list[1].connect_is_ok == 0 )
    {
        creat_udp_connect_process(1);
    }
    else if( tcp_client_list[1].tcp_client_statue == CLIENT_DIS_CONNECT )
    {
        udp_echoclient_connect(1);
    } */
    /*else if(tcp_client_list[1].tcp_client_statue == CLIENT_CONNECT_OK )
    {
        //发送
            if(tcp_client_list[1].tcp_send_en == 1)
            {
                udp_send_process( 1 );
                tcp_client_list[1].tcp_send_en = 0;
                heart_dely = 30;
            }
    }*/
}

#endif
/*******************************************************************************************

*******************************************************************************************/


void  DEBUG_process_udp(u8 *p_data, u16 len)
{

    memcpy(&(tcp_client_list[0].tcp_send_buf[0]), p_data, len);
    tcp_client_list[0].tcp_send_len = len;
    //reply_recv_msg(, , msg_type, reply_index_error);
    tcp_client_list[0].tcp_send_en = 1;

    if(tcp_client_list[0].tcp_client_statue == CLIENT_CONNECT_OK )
    {
        if(tcp_client_list[0].tcp_send_en == 1)
        {
            udp_send_process(0);
            tcp_client_list[0].tcp_send_en = 0;
            heart_dely = HEART_DELAY;
        }
    }
}





void  DEBUG_process(u8 *p_data, u16 len)
{
#ifndef     USE_UDP
    if( ( tcp_client_list[1].tcp_client_statue == CLIENT_CONNECT_OK ) && ( tcp_client_list[1].echoclient_pcb->state == ESTABLISHED) )
    {
        tcp_write(tcp_client_list[1].echoclient_pcb, p_data, len, 1);
    }
#else
    u16 i;
    if( tcp_client_list[1].tcp_client_statue == CLIENT_CONNECT_OK )
    {
        for(i = 0; i < len; i++)
        {
            tcp_client_list[1].tcp_send_buf[i] = *(p_data + i);
        }
        tcp_client_list[1].tcp_send_len = len;
        udp_send_process(1);
    }
#endif
}

/**************************************************************************

**************************************************************************/
void WCS_TCP_reply_process(u8 *p_data, u16 len)
{
#ifndef     USE_UDP
    if( tcp_client_list[0].echoclient_pcb->state == ESTABLISHED)
    {
        tcp_write(tcp_client_list[0].echoclient_pcb, p_data, len, 1);
    }
#else
    u16 i;
    if( tcp_client_list[1].tcp_client_statue == CLIENT_CONNECT_OK )
    {
        for(i = 0; i < len; i++)
        {
            tcp_client_list[1].tcp_send_buf[i] = *(p_data + i);
        }
        tcp_client_list[1].tcp_send_len = len;
        udp_send_process(1);
    }
#endif
}
/***********************************************************

***************************************************************/
void get_ip_para_process(u8 tcp_offset)
{
    if(tcp_offset == 0)
    {
        tcp_client_list[tcp_offset].dest_port = DEST_PORT;

        tcp_client_list[tcp_offset].dest_ip[0] = DEST_IP_ADDR0;
        tcp_client_list[tcp_offset].dest_ip[1] = DEST_IP_ADDR1;
        tcp_client_list[tcp_offset].dest_ip[2] = DEST_IP_ADDR2;
        tcp_client_list[tcp_offset].dest_ip[3] = DEST_IP_ADDR3;
    }
    else
    {
        tcp_client_list[tcp_offset].dest_port = 9000;

        tcp_client_list[tcp_offset].dest_ip[0] = 192;
        tcp_client_list[tcp_offset].dest_ip[1] = 168;
        tcp_client_list[tcp_offset].dest_ip[2] = 10;
        tcp_client_list[tcp_offset].dest_ip[3] = 50;
    }
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
