`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/17/2023 08:58:58 PM
// Design Name: 
// Module Name: k_do_mult
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module k_do_mult(
    parameter integer MULT_WIDTH            = 16,
    parameter integer FFTDATA_RE_WIDTH      = 16,
    parameter integer FFT_NUM_PTS           = 32
    )
    (
    input                                       clk,
    input                                       aresetn,

    input                                       fft_in_axis_tvalid,
    output wire                                 fft_in_axis_tready,
    input signed [2*FFTDATA_RE_WIDTH-1 : 0]     fft_in_axis_tdata,

    input                                       mult_in_axis_tvalid,
    output wire                                 mult_in_axis_tready,
    input signed [MULT_WIDTH-1 : 0]             mult_in_axis_tdata,

    output reg [2*FFTDATA_RE_WIDTH-1 : 0]       fft_out_axis_a_tdata,
    input                                       fft_out_axis_a_tready,
    output reg                                  fft_out_axis_a_tvalid,
    );
    localparam  FFTDATA_CNT_MAX_BITS            = 24;

    localparam  [3:0] IDLE                      = 4'b0000;
    localparam  [3:0] READ_MULTIPLIER           = 4'b0001;
    localparam  [3:0] DO_MULT                   = 4'b0010;
    localparam  [3:0] READ_FFTDATA              = 4'b0011;
    localparam  [3:0] STREAM_OUT                = 4'b0100;
    reg [3:0]                                       k_cur_state = IDLE;
    wire [FFDATA_RE_WIDTH-1 : 0]                    in_re;
    wire [FFDATA_RE_WIDTH-1 : 0]                    in_im;
    reg signed [FFDATA_RE_WIDTH-1 : 0]              re_reg;
    reg signed [FFDATA_RE_WIDTH-1 : 0]              im_reg;
    reg signed [MULT_WIDTH-1 : 0]                   mult_reg;
    reg signed [FFDATA_RE_WIDTH-1 : 0]              mult_re_reg;
    reg signed [FFDATA_RE_WIDTH-1 : 0]              mult_im_reg;
    reg [OUT_WIDTH      : 0]                        out_reg;
    reg unsigned [FFTDATA_CNT_MAX_BITS-1 : 0]       FFTdata_cnt;

    assign in_re [ FFDATA_RE_WIDTH-1 : 0 ] = fft_in_axis_tdata [ 2*FFDATA_RE_WIDTH-1   : FFDATA_RE_WIDTH ];
    assign in_im [ FFDATA_RE_WIDTH-1 : 0 ] = fft_in_axis_tdata [ FFDATA_RE_WIDTH-1     : 0];
    

    assign mult_in_axis_tready = (k_cur_state == READ_MULTIPLIER);
    assign fft_in_axis_tready  = (k_cur_state == READ_FFTDATA);
    assign fft_out_axis_tready = (k_cur_state == STREAM_OUT);

    // STATE MACHINE
    always @(posedge clk)
    begin
        if (!aresetn)
        begin
            k_cur_state <= IDLE;
        end
        else
        begin
            case (k_cur_state)
                IDLE:
                begin
                    FFTdata_cnt <= 0;
                    k_cur_state <= READ_MULTIPLIER;
                end

                READ_MULTIPLIER:
                begin
                    if (mult_in_axis_tvalid && mult_in_axis_tready) begin
                        mult_reg <= mult_in_axis_tdata;
                        k_cur_state <= READ_FFTDATA;
                    end
                    else begin
                        k_cur_state <= k_cur_state;
                    end
                end

                READ_FFTDATA:
                begin
                    if (fft_in_axis_tvalid && fft_in_axis_tready) begin
                        re_reg <= in_re;
                        im_reg <= in_im;
                        FFTdata_cnt <= FFTdata_cnt + 1;
                        k_cur_state <= DO_MULT;
                    end
                    else begin
                        k_cur_state <= k_cur_state;
                    end
                end

                DO_MULT:
                begin
                    mult_re_reg <= re_reg * mult_reg;
                    mult_im_reg <= im_reg * mult_reg;
                    k_cur_state <= STREAM_OUT;
                end

                STREAM_OUT:
                begin
                    if (fft_out_axis_tvalid && fft_out_axis_tready) begin
                        if (FFTdata_cnt < FFT_NUM_PTS) begin
                            k_cur_state <= READ_FFTDATA;
                        end
                        else if (FFTdata_cnt == FFT_NUM_PTS) begin
                            k_cur_state <= IDLE;
                        end
                        else begin
                            k_cur_state <= IDLE;
                        end
                    end
                    else begin
                        k_cur_state <= k_cur_state;
                    end
                end

                default:
                begin
                    k_next_state <= k_cur_state;
                end
            endcase
        end
    end

endmodule
