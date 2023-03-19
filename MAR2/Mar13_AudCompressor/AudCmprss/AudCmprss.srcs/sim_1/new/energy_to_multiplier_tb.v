`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/13/2023 05:28:19 PM
// Design Name: 
// Module Name: energy_to_multiplier_tb
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


module energy_to_multiplier_tb();
    parameter DATA_WIDTH = 16;
    parameter OUT_WIDTH = 16;
    parameter FFT_NUM_PTS = 32;

    reg                           aclk;
    wire                          aresetn;

    reg  [DATA_WIDTH-1:0]         fftdata_re_in;
    reg  [DATA_WIDTH-1:0]         fftdata_im_in;

    reg   [2*DATA_WIDTH-1:0]      fftdata_data_in;
    reg                           fftdata_valid;
    wire                          fftdata_ready;

    reg [OUT_WIDTH-1:0]           output_value;

    wire  [OUT_WIDTH-1:0]         output_WIRE;
    wire                          output_valid;
    wire                          output_ready;

    reg   [DATA_WIDTH-1:0]        re_data[0:FFT_NUM_PTS-1]; //[c,n,s,e,w]
    reg   [DATA_WIDTH-1:0]        im_data[0:FFT_NUM_PTS-1];

    ///
    assign output_ready = 1;
    assign aresetn = 1;


    integer temp_counter;
    initial begin

        // Matlab generated data with rng(2), multiplied by 15:
        re_data[0   ] = 16'h0007;
        re_data[1   ] = 16'h0008;
        re_data[2   ] = 16'h0006;
        re_data[3   ] = 16'h0003;
        re_data[4   ] = 16'h0004;
        re_data[5   ] = 16'h0009;
        re_data[6   ] = 16'h0002;
        re_data[7   ] = 16'h0003;
        re_data[8   ] = 16'h000D;
        re_data[9   ] = 16'h000D;
        re_data[10  ] = 16'h0008;
        re_data[11  ] = 16'h0006;
        re_data[12  ] = 16'h0002;
        re_data[13  ] = 16'h0003;
        re_data[14  ] = 16'h0003;
        re_data[15  ] = 16'h0007;
        re_data[16  ] = 16'h000A;
        re_data[17  ] = 16'h0008;
        re_data[18  ] = 16'h000C;
        re_data[19  ] = 16'h0002;
        re_data[20  ] = 16'h000E;
        re_data[21  ] = 16'h000D;
        re_data[22  ] = 16'h0009;
        re_data[23  ] = 16'h0007;
        re_data[24  ] = 16'h0008;
        re_data[25  ] = 16'h0008;
        re_data[26  ] = 16'h0005;
        re_data[27  ] = 16'h0006;
        re_data[28  ] = 16'h0004;
        re_data[29  ] = 16'h000F;
        re_data[30  ] = 16'h000C;
        re_data[31  ] = 16'h000B;

        im_data[0   ] = 16'h0000;
        im_data[1   ] = 16'h0007;
        im_data[2   ] = 16'h0005;
        im_data[3   ] = 16'h0009;
        im_data[4   ] = 16'h0004;
        im_data[5   ] = 16'h0008;
        im_data[6   ] = 16'h0008;
        im_data[7   ] = 16'h000C;
        im_data[8   ] = 16'h0007;
        im_data[9   ] = 16'h0001;
        im_data[10  ] = 16'h0001;
        im_data[11  ] = 16'h0001;
        im_data[12  ] = 16'h0009;
        im_data[13  ] = 16'h0002;
        im_data[14  ] = 16'h0005;
        im_data[15  ] = 16'h0003;
        im_data[16  ] = 16'h0007;
        im_data[17  ] = 16'h0006;
        im_data[18  ] = 16'h0009;
        im_data[19  ] = 16'h000B;
        im_data[20  ] = 16'h0008;
        im_data[21  ] = 16'h0005;
        im_data[22  ] = 16'h0006;
        im_data[23  ] = 16'h000C;
        im_data[24  ] = 16'h000E;
        im_data[25  ] = 16'h0001;
        im_data[26  ] = 16'h000D;
        im_data[27  ] = 16'h0000;
        im_data[28  ] = 16'h0001;
        im_data[29  ] = 16'h000F;
        im_data[30  ] = 16'h0009;
        im_data[31  ] = 16'h0003;

        // Matlab generated data with rng(1), multiplied by 100:
        //re_data[0   ] = 16'h002A;
        //re_data[1   ] = 16'h0000;
        //re_data[2   ] = 16'h000F;
        //re_data[3   ] = 16'h0013;
        //re_data[4   ] = 16'h0028;
        //re_data[5   ] = 16'h002A;
        //re_data[6   ] = 16'h0014;
        //re_data[7   ] = 16'h0003;
        //re_data[8   ] = 16'h002A;
        //re_data[9   ] = 16'h000E;
        //re_data[10  ] = 16'h0050;
        //re_data[11  ] = 16'h001F;
        //re_data[12  ] = 16'h0058;
        //re_data[13  ] = 16'h0009;
        //re_data[14  ] = 16'h0011;
        //re_data[15  ] = 16'h000A;
        //re_data[16  ] = 16'h0060;
        //re_data[17  ] = 16'h0045;
        //re_data[18  ] = 16'h0045;
        //re_data[19  ] = 16'h0002;
        //re_data[20  ] = 16'h0063;
        //re_data[21  ] = 16'h001C;
        //re_data[22  ] = 16'h000A;
        //re_data[23  ] = 16'h005B;
        //re_data[24  ] = 16'h001D;
        //re_data[25  ] = 16'h0002;
        //re_data[26  ] = 16'h0015;
        //re_data[27  ] = 16'h0031;
        //re_data[28  ] = 16'h0039;
        //re_data[29  ] = 16'h003B;
        //re_data[30  ] = 16'h000A;
        //re_data[31  ] = 16'h0045;

        //im_data[0   ] = 16'h0048;
        //im_data[1   ] = 16'h001E;
        //im_data[2   ] = 16'h0009;
        //im_data[3   ] = 16'h0023;
        //im_data[4   ] = 16'h0036;
        //im_data[5   ] = 16'h0045;
        //im_data[6   ] = 16'h0058;
        //im_data[7   ] = 16'h0043;
        //im_data[8   ] = 16'h0038;
        //im_data[9   ] = 16'h0014;
        //im_data[10  ] = 16'h0061;
        //im_data[11  ] = 16'h0045;
        //im_data[12  ] = 16'h0059;
        //im_data[13  ] = 16'h0004;
        //im_data[14  ] = 16'h0058;
        //im_data[15  ] = 16'h002A;
        //im_data[16  ] = 16'h0035;
        //im_data[17  ] = 16'h0020;
        //im_data[18  ] = 16'h0053;
        //im_data[19  ] = 16'h004B;
        //im_data[20  ] = 16'h004B;
        //im_data[21  ] = 16'h004F;
        //im_data[22  ] = 16'h002D;
        //im_data[23  ] = 16'h001D;
        //im_data[24  ] = 16'h000D;
        //im_data[25  ] = 16'h0044;
        //im_data[26  ] = 16'h001B;
        //im_data[27  ] = 16'h0005;
        //im_data[28  ] = 16'h000F;
        //im_data[29  ] = 16'h0046;
        //im_data[30  ] = 16'h0029;
        //im_data[31  ] = 16'h0029;

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
        // Pack the data
        fftdata_data_in [2*DATA_WIDTH - 1 : DATA_WIDTH]  <= fftdata_re_in[DATA_WIDTH - 1 : 0];
        fftdata_data_in [DATA_WIDTH - 1 : 0]             <= fftdata_im_in[DATA_WIDTH - 1 : 0];

        if(fftdata_ready && fftdata_valid) begin
            fftdata_re_in <= re_data[temp_counter];
            fftdata_im_in <= im_data[temp_counter];
            if(temp_counter == FFT_NUM_PTS- 1) 
                temp_counter = 0;
            else
                temp_counter=temp_counter+1;
        end
    end
    
    //control validity of temp data
    initial begin
        fftdata_valid = 0;
        fftdata_re_in = re_data[0];
        fftdata_im_in = im_data[0];
        #24;
        fftdata_valid = 1;
        #1;
        fftdata_valid = 0;
        #16;
        fftdata_valid = 0;
        #8;
        fftdata_valid = 1;
        #20;
        fftdata_valid = 0;
        #8;
        fftdata_valid = 1;
        #20;
        fftdata_valid = 0;
        #8;
        fftdata_valid = 1;
    end

    // Reading output data
    always @(posedge aclk) begin
        if (output_valid) begin
            output_value <= output_WIRE;
        end
    end
    
    //simulation time handle
    initial begin
        #900;
        $finish;
    end


    full_compressor full_compressor_i (
        .M_AXIS_RESULT_0_tdata(output_WIRE),
        .M_AXIS_RESULT_0_tready(output_ready),
        .M_AXIS_RESULT_0_tvalid(output_valid),
        .aresetn_0(aresetn),
        .clk_0(aclk),
        .s_axis_0_tdata(fftdata_data_in),
        .s_axis_0_tready(fftdata_ready),
        .s_axis_0_tvalid(fftdata_valid)
    );

    //design_1 design_1_i (
    //    .M_AXIS_RESULT_0_tdata(output_WIRE),
    //    .M_AXIS_RESULT_0_tready(output_ready),
    //    .M_AXIS_RESULT_0_tvalid(output_valid),
    //    .aresetn_0(aresetn),
    //    .clk_0(aclk),
    //    .s_axis_0_tdata(fftdata_data_in),
    //    .s_axis_0_tready(fftdata_ready),
    //    .s_axis_0_tvalid(fftdata_valid)
    //);

endmodule
