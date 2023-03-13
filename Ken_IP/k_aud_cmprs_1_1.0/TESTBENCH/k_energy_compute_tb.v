`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/12/2023 06:07:52 PM
// Design Name: 
// Module Name: k_tb
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


module k_tb();
    parameter DATA_WIDTH = 16;
    parameter OUT_WIDTH = 40;
    parameter FFT_NUM_PTS = 16;

    reg                           aclk;

    reg   [DATA_WIDTH-1:0]        energy_re_in;
    reg   [DATA_WIDTH-1:0]        energy_im_in;
    wire   [OUT_WIDTH-1:0]         energy_out;
    wire                           energy_out_valid;

    reg   [DATA_WIDTH-1:0]        re_data[0:FFT_NUM_PTS-1]; //[c,n,s,e,w]
    reg   [DATA_WIDTH-1:0]        im_data[0:FFT_NUM_PTS-1];

    integer temp_counter;
    initial begin
        re_data[0 ] = 16'h0034;
        re_data[1 ] = 16'hFFE8;
        re_data[2 ] = 16'h0017;
        re_data[3 ] = 16'hFFE4;
        re_data[4 ] = 16'hFFE8;
        re_data[5 ] = 16'h0017;
        re_data[6 ] = 16'hFFE4;
        re_data[7 ] = 16'h0026;
        re_data[8 ] = 16'h0060;
        re_data[9 ] = 16'h0001;
        re_data[10] = 16'h0031;
        re_data[11] = 16'h0034;
        re_data[12] = 16'h8000;
        re_data[13] = 16'hFFFF;
        re_data[14] = 16'h7FFF;
        re_data[15] = 16'h0000;

        im_data[0 ] = 16'h0006;
        im_data[1 ] = 16'h0063;
        im_data[2 ] = 16'hFF9D;
        im_data[3 ] = 16'h0010;
        im_data[4 ] = 16'h0063;
        im_data[5 ] = 16'hFF9D;
        im_data[6 ] = 16'h0010;
        im_data[7 ] = 16'h002C;
        im_data[8 ] = 16'h0022;
        im_data[9 ] = 16'h0019;
        im_data[10] = 16'hFFE7;
        im_data[11] = 16'h0006;
        im_data[12] = 16'h8000;
        im_data[13] = 16'hFFFF;
        im_data[14] = 16'h7FFF;
        im_data[15] = 16'h0000;
        temp_counter = 0;     
    end

    //clock handle
    initial begin
        aclk = 0;
        forever #2 aclk = ~aclk;
    end
    
    //passing temp data
    always @(negedge aclk) begin
        if(~energy_out_valid) begin
            energy_re_in <= energy_re_in;
            energy_im_in <= energy_im_in;
        end
        else begin
            energy_re_in <= re_data[temp_counter];
            energy_im_in <= im_data[temp_counter];
            if (temp_counter == FFT_NUM_PTS-1)
                temp_counter = 0;
            else 
                temp_counter = temp_counter+1;


            //if(s_axis_temp_valid) begin
            //    s_axis_temp_data <= temp_data[temp_counter];
            //    if(temp_counter == 9) 
            //        temp_counter = 0;
            //    else
            //        temp_counter=temp_counter+1;
            //end
        end
    end
    
    //control validity of temp data
    initial begin
//        s_axis_temp_valid = 0;
//        #24;
//        s_axis_temp_valid = 1;
//        s_axis_temp_data <= temp_data[0];
//        #1;
//        s_axis_temp_valid = 0;
//        #16;
//        s_axis_temp_valid = 0;
//        #8;
//        s_axis_temp_valid = 1;
//        #20;
//        s_axis_temp_valid = 0;
//        #8;
//        s_axis_temp_valid = 1;
//        #20;
//        s_axis_temp_valid = 0;
//        #8;
//        s_axis_temp_valid = 1;
//        #68;
//        s_axis_temp_valid = 0;
    end

    
    //simulation time handle
    initial begin
        #400;
        $finish;
    end





    k_energy_computer # (
        .IN_WIDTH(DATA_WIDTH),
        .OUT_WIDTH(OUT_WIDTH)
    ) energy_cmp_inst (
        .clk(aclk),
        .in_re(energy_re_in),
        .in_im(energy_im_in),
        .out_energy(energy_out),
        .out_valid(energy_out_valid)
    );
    
endmodule
