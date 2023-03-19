`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/12/2023 05:10:15 PM
// Design Name: 
// Module Name: k_energy_computer
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


module k_energy_computer #
    (
    parameter integer IN_WIDTH = 16,
    parameter integer OUT_WIDTH = 40
    )
    (
    input                           clk,
    input                           s_axis_tvalid,
    output wire                     s_axis_tready,
    input signed [2*IN_WIDTH-1 : 0] s_axis_tdata,
    output [OUT_WIDTH-1 : 0]        out_energy,
    output reg                      out_valid
    );

    localparam  [3:0] IDLE                    = 4'b0000;
    localparam  [3:0] COMPUTE_SQUARES         = 4'b0001;
    localparam  [3:0] ADD_SQUARES             = 4'b0010;
    localparam  [3:0] DONE                    = 4'b0011;
    reg [3:0] k_cur_state   = IDLE;
    reg [2*IN_WIDTH-1   : 0]    re_sqrd;
    reg [2*IN_WIDTH-1   : 0]    im_sqrd;
    reg [OUT_WIDTH      : 0]    out_reg;
    wire [IN_WIDTH-1 : 0]      in_re;
    wire [IN_WIDTH-1 : 0]      in_im;
    reg signed [IN_WIDTH-1 : 0]      in_re_reg;
    reg signed [IN_WIDTH-1 : 0]      in_im_reg;

    assign in_re [ IN_WIDTH-1 : 0 ] = s_axis_tdata [ 2*IN_WIDTH-1   : IN_WIDTH ];
    assign in_im [ IN_WIDTH-1 : 0 ] = s_axis_tdata [ IN_WIDTH-1     : 0];
    
    //integer i;
    //initial begin
    //    for (i = 0; i < 2*IN_WIDTH; i=i+1)
    //    begin
    //        re_sqrd[i] = 1'b0;
    //        im_sqrd[i] = 1'b0;
    //    end
    //    for (i = 0; i < OUT_WIDTH; i=i+1)
    //    begin
    //        out_reg[i] = 1'b0;
    //    end
    //end

    assign out_energy = out_reg;

    // STATE MACHINE
    always @(posedge clk)
    begin
        case (k_cur_state)
            IDLE:
            begin
                if (s_axis_tvalid && s_axis_tready) begin
                    in_re_reg <= in_re;
                    in_im_reg <= in_im;
                    k_cur_state <= COMPUTE_SQUARES;
                end
                else begin
                    k_cur_state <= IDLE; 
                end
            end

            COMPUTE_SQUARES:
            begin
                re_sqrd <= in_re_reg * in_re_reg;
                im_sqrd <= in_im_reg * in_im_reg;
                k_cur_state <= ADD_SQUARES;
            end

            ADD_SQUARES:
            begin
                out_reg <= re_sqrd + im_sqrd;
                k_cur_state <= DONE;
            end

            DONE:
            begin
                k_cur_state <= IDLE;
            end

        endcase
    end

    // READY SIGNAL:
    assign s_axis_tready = (k_cur_state == IDLE);

    //// Next State Logic
    //always @(*)
    //begin
    //    k_next_state = k_cur_state;
    //    case (k_cur_state)
    //        IDLE:
    //            if (s_axis_tvalid && s_axis_tready) begin
    //                k_next_state = COMPUTE_SQUARES;
    //            end
    //            else begin
    //                k_next_state = IDLE;
    //            end
    //        COMPUTE_SQUARES:
    //            k_next_state = ADD_SQUARES;
    //        ADD_SQUARES:
    //            k_next_state = DONE;
    //        DONE:
    //            k_next_state = IDLE;
    //    endcase
    //end


    always@(posedge clk )
    begin
        if (k_cur_state == DONE) begin
            out_valid <= 1'b1;
        end
        else begin
            out_valid <= 1'b0;
        end
    end



endmodule
