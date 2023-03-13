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
    input                       clk,
    input signed [IN_WIDTH-1 : 0]      in_re,
    input signed [IN_WIDTH-1 : 0]      in_im,
    output [OUT_WIDTH-1 : 0]    out_energy,
    output reg                  out_valid
    );

    parameter [3:0] IDLE                    = 4'b0000,
                    COMPUTE_SQUARES         = 4'b0001,
                    ADD_SQUARES             = 4'b0010,
                    DONE                    = 4'b0011;
    reg [3:0] k_cur_state   = IDLE;
    reg [3:0] k_next_state  = IDLE;
    reg [2*IN_WIDTH-1   : 0]    re_sqrd;
    reg [2*IN_WIDTH-1   : 0]    im_sqrd;
    reg [OUT_WIDTH      : 0]    out_reg;
    reg [IN_WIDTH-1 : 0]        prev_in_re;
    reg [IN_WIDTH-1 : 0]        prev_in_im;
    wire                        new_inputs;
    
    integer i;
    initial begin
        for (i = 0; i < IN_WIDTH; i=i+1)
        begin
            prev_in_re[i] = 1'b0;
            prev_in_im[i] = 1'b0;
        end
        for (i = 0; i < 2*IN_WIDTH; i=i+1)
        begin
            re_sqrd[i] = 1'b0;
            im_sqrd[i] = 1'b0;
        end
        for (i = 0; i < OUT_WIDTH; i=i+1)
        begin
            out_reg[i] = 1'b0;
        end
    end

    assign new_inputs = (prev_in_re != in_re) || (prev_in_im != in_im);
    assign out_energy = out_reg;

    always@(posedge clk )
    begin
        k_cur_state <= k_next_state;
        prev_in_re <= in_re;
        prev_in_im <= in_im;
    end

    // STATE MACHINE
    always @(posedge clk)
    begin
        case (k_cur_state)
            COMPUTE_SQUARES:
            begin
                re_sqrd <= in_re * in_re;
                im_sqrd <= in_im * in_im;
            end

            ADD_SQUARES:
                out_reg <= re_sqrd + im_sqrd;

            default:
                out_reg <= out_reg;

        endcase
    end

    // Next State Logic
    always @(*)
    begin
        case (k_cur_state)
            IDLE:
                if (new_inputs) begin
                    k_next_state <= COMPUTE_SQUARES;
                end
                else begin
                    k_next_state <= IDLE;
                end
            COMPUTE_SQUARES:
                k_next_state <= ADD_SQUARES;
            ADD_SQUARES:
                k_next_state <= DONE;
            DONE:
                k_next_state <= IDLE;
        endcase
    end


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
