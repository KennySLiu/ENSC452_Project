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


module k_energy_compute_tb();
    parameter DATA_WIDTH = 16;
    parameter OUT_WIDTH = 40;
    parameter FFT_NUM_PTS = 16;

    reg                           aclk;

    reg  [DATA_WIDTH-1:0]         energy_re_in;
    reg  [DATA_WIDTH-1:0]         energy_im_in;

    reg   [2*DATA_WIDTH-1:0]      energy_data_in;
    reg                           energy_valid;
    wire                          energy_ready;

    reg [OUT_WIDTH-1:0]         energy_output_value;

    wire  [OUT_WIDTH-1:0]         energy_out_WIRE;
    wire                          energy_out_valid;

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
    //always @(negedge aclk) begin
    always @(posedge aclk) begin
        energy_data_in [2*DATA_WIDTH - 1 : DATA_WIDTH]  <= energy_re_in[DATA_WIDTH - 1 : 0];
        energy_data_in [DATA_WIDTH - 1 : 0]             <= energy_im_in[DATA_WIDTH - 1 : 0];

        if(energy_ready && energy_valid) begin
            energy_re_in <= re_data[temp_counter];
            energy_im_in <= im_data[temp_counter];
            if(temp_counter == FFT_NUM_PTS- 1) 
                temp_counter = 0;
            else
                temp_counter=temp_counter+1;
        end
    end
    
    //control validity of temp data
    initial begin
        energy_valid = 0;
        energy_re_in = re_data[0];
        energy_im_in = im_data[0];
        #24;
        energy_valid = 1;
        #1;
        energy_valid = 0;
        #16;
        energy_valid = 0;
        #8;
        energy_valid = 1;
        #20;
        energy_valid = 0;
        #8;
        energy_valid = 1;
        #20;
        energy_valid = 0;
        #8;
        energy_valid = 1;
    end

    // Reading output data
    always @(posedge aclk) begin
        if (energy_valid) begin
            energy_output_value <= energy_out_WIRE;
        end
    end
    
    //simulation time handle
//    initial begin
//        #600;
//        $finish;
//    end





    k_energy_computer # (
        .IN_WIDTH(DATA_WIDTH),
        .OUT_WIDTH(OUT_WIDTH)
    ) energy_cmp_inst (
        .clk(aclk),
        .s_axis_tvalid(energy_valid),
        .s_axis_tready(energy_ready),
        .s_axis_tdata (energy_data_in),
        .out_energy(energy_out_WIRE),
        .out_valid(energy_out_valid)
    );
    
endmodule
