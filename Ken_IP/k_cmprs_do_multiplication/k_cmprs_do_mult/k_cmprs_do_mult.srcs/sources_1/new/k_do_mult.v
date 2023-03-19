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


module k_do_mult # (
    parameter integer MULT_WIDTH            = 16,
    parameter integer MULT_FRACT_BITS       = 8,
    parameter integer FFTDATA_RE_WIDTH      = 16,
    parameter integer NUM_FFT_PTS           = 16
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

    output wire [2*FFTDATA_RE_WIDTH-1 : 0]      fft_out_axis_tdata,
    input                                       fft_out_axis_tready,
    output wire                                 fft_out_axis_tvalid
    );

    ////////////////////////////////////////////////////////////////
    //// Signals
    ////////////////////////////////////////////////////////////////

    // Just make sure it's big enough...
    localparam  FFTDATA_CNT_MAX_BITS                = 24;
    localparam  [3:0] IDLE                          = 4'b0000;
    localparam  [3:0] READ_MULTIPLIER               = 4'b0001;
    localparam  [3:0] DO_MULT                       = 4'b0010;
    localparam  [3:0] READ_FFTDATA                  = 4'b0011;
    localparam  [3:0] STREAM_OUT                    = 4'b0100;

    localparam  MULT_INT_BITS                       = MULT_WIDTH - MULT_FRACT_BITS;
    localparam  MULTIPLIED_RES_BITS                 = MULT_FRACT_BITS + FFTDATA_RE_WIDTH;
    localparam  [FFTDATA_RE_WIDTH - MULT_INT_BITS -1 : 0] MULTIPLIER_PADDING_BITS = 'h0;
    localparam  [MULT_FRACT_BITS-1 : 0]  FFTDATA_PADDING_BITS = 'h0;

    wire [MULT_INT_BITS - 1 : 0]                    multval_int_bits;

    reg [3:0]                                       k_cur_state = IDLE;
    wire [FFTDATA_RE_WIDTH-1 : 0]                   in_re;
    wire [FFTDATA_RE_WIDTH-1 : 0]                   in_im;
    reg signed [FFTDATA_RE_WIDTH-1 : 0]             re_reg;
    reg signed [FFTDATA_RE_WIDTH-1 : 0]             im_reg;
    reg signed [MULT_WIDTH-1 : 0]                   multval_reg;

    wire signed [MULTIPLIED_RES_BITS-1 : 0]         multiplier_shifted;
    wire signed [MULTIPLIED_RES_BITS-1 : 0]         re_value_shifted;
    wire signed [MULTIPLIED_RES_BITS-1 : 0]         im_value_shifted;
    reg signed [2*MULTIPLIED_RES_BITS-1 : 0]        re_result_shifted_reg;
    reg signed [2*MULTIPLIED_RES_BITS-1 : 0]        im_result_shifted_reg;

    wire signed [FFTDATA_RE_WIDTH-1 : 0]            re_result;
    wire signed [FFTDATA_RE_WIDTH-1 : 0]            im_result;
    reg signed [FFTDATA_CNT_MAX_BITS-1 : 0]         FFTdata_cnt;


    ////////////////////////////////////////////////////////////////
    //// Logic
    ////////////////////////////////////////////////////////////////


    assign in_re [ FFTDATA_RE_WIDTH-1 : 0 ] = fft_in_axis_tdata [ 2*FFTDATA_RE_WIDTH-1   : FFTDATA_RE_WIDTH ];
    assign in_im [ FFTDATA_RE_WIDTH-1 : 0 ] = fft_in_axis_tdata [ FFTDATA_RE_WIDTH-1     : 0];
    
    assign multval_int_bits[MULT_INT_BITS - 1 : 0] =    multval_reg [MULT_WIDTH - 1 : MULT_FRACT_BITS];

    assign mult_in_axis_tready  = (k_cur_state == READ_MULTIPLIER);
    assign fft_in_axis_tready   = (k_cur_state == READ_FFTDATA);
    assign fft_out_axis_tvalid  = (k_cur_state == STREAM_OUT);
    assign fft_out_axis_tdata [2*FFTDATA_RE_WIDTH-1 : FFTDATA_RE_WIDTH]     = re_result[FFTDATA_RE_WIDTH-1 : 0];
    assign fft_out_axis_tdata [FFTDATA_RE_WIDTH-1 : 0]                      = im_result[FFTDATA_RE_WIDTH-1 : 0];

    // To do the fixed-pt multiplication, we need to shift the bits:
    // We need both operands to have N bits of integer and M bits of fractional.
    // And the result will have 2N bits of integer and 2M bits of fractional.
    assign re_value_shifted     = {re_reg, FFTDATA_PADDING_BITS};
    assign im_value_shifted     = {im_reg, FFTDATA_PADDING_BITS};
    assign multiplier_shifted   = {MULTIPLIER_PADDING_BITS, multval_reg};
    assign re_result [FFTDATA_RE_WIDTH-1 : 0] = re_result_shifted_reg [2*MULT_FRACT_BITS+FFTDATA_RE_WIDTH-1 : 2*MULT_FRACT_BITS];
    assign im_result [FFTDATA_RE_WIDTH-1 : 0] = im_result_shifted_reg [2*MULT_FRACT_BITS+FFTDATA_RE_WIDTH-1 : 2*MULT_FRACT_BITS];

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
                        multval_reg <= mult_in_axis_tdata;
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
                    if (multval_int_bits == 'b0) begin
                        re_result_shifted_reg <= re_value_shifted * multiplier_shifted;
                        im_result_shifted_reg <= im_value_shifted * multiplier_shifted;
                    end
                    else begin
                        // Don't multiply - just copy the original values.
                        re_result_shifted_reg [2*MULT_FRACT_BITS+FFTDATA_RE_WIDTH-1 : 2*MULT_FRACT_BITS] <= re_value_shifted [MULTIPLIED_RES_BITS - 1 : MULT_FRACT_BITS];
                        im_result_shifted_reg [2*MULT_FRACT_BITS+FFTDATA_RE_WIDTH-1 : 2*MULT_FRACT_BITS] <= im_value_shifted [MULTIPLIED_RES_BITS - 1 : MULT_FRACT_BITS];
                    end
                    k_cur_state <= STREAM_OUT;
                end

                STREAM_OUT:
                begin
                    if (fft_out_axis_tvalid && fft_out_axis_tready) begin
                        if (FFTdata_cnt < NUM_FFT_PTS) begin
                            k_cur_state <= READ_FFTDATA;
                        end
                        else if (FFTdata_cnt == NUM_FFT_PTS) begin
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
                    k_cur_state <= k_cur_state;
                end
            endcase
        end
    end

endmodule
