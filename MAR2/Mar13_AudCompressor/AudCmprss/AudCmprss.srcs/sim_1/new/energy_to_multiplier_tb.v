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
    parameter OUT_WIDTH = 32;
    parameter MAX_FFT_PTS = 2048;

    reg                             aclk;
    wire                            aresetn;

    reg  [DATA_WIDTH-1:0]           fftdata_re_in;
    reg  [DATA_WIDTH-1:0]           fftdata_im_in;

    wire [2*DATA_WIDTH-1:0]         fftdata_data_in;
    reg                             fftdata_valid;
    wire                            fftdata_ready;

    reg [OUT_WIDTH-1:0]             output_value;

    wire  [OUT_WIDTH-1:0]           output_WIRE;
    wire                            output_valid;
    wire                            output_ready;

    reg   [DATA_WIDTH-1:0]          re_data[0:MAX_FFT_PTS-1];
    reg   [DATA_WIDTH-1:0]          im_data[0:MAX_FFT_PTS-1];

    reg   [DATA_WIDTH-1:0]          output_re_value;
    reg   [DATA_WIDTH-1:0]          output_im_value;

    wire  [31:0]                    Thresh;
    wire  [31:0]                    Ratio;
    wire  [24:0]                    num_fft_pts;

    ///
    assign output_ready = 1;
    assign aresetn = 1;

    // Pack the data
    assign fftdata_data_in [2*DATA_WIDTH - 1 : DATA_WIDTH]  = fftdata_re_in[DATA_WIDTH - 1 : 0];
    assign fftdata_data_in [DATA_WIDTH - 1 : 0]             = fftdata_im_in[DATA_WIDTH - 1 : 0];

    assign Thresh       = 4000;
    assign Ratio        = 15;
    assign num_fft_pts  = 32;



    integer temp_counter;
    initial begin

        // Matlab generated data with rng(2), (rand-0.5), multiplied by 1000:
        re_data[0   ] = 16'hFFC0;
        re_data[1   ] = 16'h0032;
        re_data[2   ] = 16'hFFB0;
        re_data[3   ] = 16'hFED9;
        re_data[4   ] = 16'hFF38;
        re_data[5   ] = 16'h0079;
        re_data[6   ] = 16'hFE93;
        re_data[7   ] = 16'hFEC4;
        re_data[8   ] = 16'h0162;
        re_data[9   ] = 16'h015B;
        re_data[10  ] = 16'h0005;
        re_data[11  ] = 16'hFFB8;
        re_data[12  ] = 16'hFE8B;
        re_data[13  ] = 16'hFEEE;
        re_data[14  ] = 16'hFEE8;
        re_data[15  ] = 16'hFFE0;
        re_data[16  ] = 16'h008C;
        re_data[17  ] = 16'h0005;
        re_data[18  ] = 16'h0126;
        re_data[19  ] = 16'hFEAE;
        re_data[20  ] = 16'h01D1;
        re_data[21  ] = 16'h0186;
        re_data[22  ] = 16'h0043;
        re_data[23  ] = 16'hFFC1;
        re_data[24  ] = 16'h0024;
        re_data[25  ] = 16'h002C;
        re_data[26  ] = 16'hFF7A;
        re_data[27  ] = 16'hFFA2;
        re_data[28  ] = 16'hFF03;
        re_data[29  ] = 16'h01EE;
        re_data[30  ] = 16'h012C;
        re_data[31  ] = 16'h0109;

        im_data[0   ] = 16'hFE26;
        im_data[1   ] = 16'hFFBF;
        im_data[2   ] = 16'hFF56;
        im_data[3   ] = 16'h0077;
        im_data[4   ] = 16'hFF17;
        im_data[5   ] = 16'h001D;
        im_data[6   ] = 16'h000E;
        im_data[7   ] = 16'h011D;
        im_data[8   ] = 16'hFFFA;
        im_data[9   ] = 16'hFE5C;
        im_data[10  ] = 16'hFE4D;
        im_data[11  ] = 16'hFE6D;
        im_data[12  ] = 16'h0061;
        im_data[13  ] = 16'hFE77;
        im_data[14  ] = 16'hFF6A;
        im_data[15  ] = 16'hFED6;
        im_data[16  ] = 16'hFFEF;
        im_data[17  ] = 16'hFF8F;
        im_data[18  ] = 16'h0050;
        im_data[19  ] = 16'h00C9;
        im_data[20  ] = 16'h0000;
        im_data[21  ] = 16'hFF62;
        im_data[22  ] = 16'hFFB8;
        im_data[23  ] = 16'h0115;
        im_data[24  ] = 16'h01C6;
        im_data[25  ] = 16'hFE5E;
        im_data[26  ] = 16'h015F;
        im_data[27  ] = 16'hFE27;
        im_data[28  ] = 16'hFE4F;
        im_data[29  ] = 16'h01D7;
        im_data[30  ] = 16'h0066;
        im_data[31  ] = 16'hFEB5;

        temp_counter = 1;
    end

    //clock handle
    initial begin
        aclk = 0;
        forever #2 aclk = ~aclk;
    end
    
    //passing temp data
    //always @(negedge aclk) begin
    always @(posedge aclk) begin
        if(fftdata_ready && fftdata_valid) begin
            fftdata_re_in <= re_data[temp_counter];
            fftdata_im_in <= im_data[temp_counter];
            if(temp_counter == num_fft_pts- 1) 
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
            //output_value <= output_WIRE;
            output_re_value [DATA_WIDTH-1 : 0] <= output_WIRE [OUT_WIDTH-1 : DATA_WIDTH];
            output_im_value [DATA_WIDTH-1 : 0] <= output_WIRE [DATA_WIDTH-1 : 0];
        end
    end
    
    //simulation time handle
    initial begin
        #1600;
        $finish;
    end


    full_compressor full_compressor_i (
        .aresetn(aresetn),
        .CLK(aclk),
        .M_AXIS_RESULT_0_tdata(output_WIRE),
        .M_AXIS_RESULT_0_tready(output_ready),
        .M_AXIS_RESULT_0_tvalid(output_valid),
        .s_axis_0_tdata(fftdata_data_in),
        .s_axis_0_tready(fftdata_ready),
        .s_axis_0_tvalid(fftdata_valid),
        .Thresh_0(Thresh),
        .Ratio_0(Ratio),
        .num_fft_pts_0(num_fft_pts)
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
