export const passThroughshaderCode = `
      struct VertexShaderOutput {
        @builtin(position) position: vec4f,
        @location(0) texcoord: vec2f,
      };

      @vertex fn vs(
        @builtin(vertex_index) vertexIndex : u32
      ) -> VertexShaderOutput {
        let uv = array(
          // 1st triangle
          vec2f( 0.0,  0.0),  // center
          vec2f( 1.0,  0.0),  // right, center
          vec2f( 0.0,  1.0),  // center, top

          // 2nd triangle
          vec2f( 0.0,  1.0),  // center, top
          vec2f( 1.0,  0.0),  // right, center
          vec2f( 1.0,  1.0),  // right, top
        );

        let pos = array(
          // 1st triangle
          vec2f( -1.0,  -1.0),  // center
          vec2f( 1.0,  -1.0),  // right, center
          vec2f( -1.0,  1.0),  // center, top

          // 2nd triangle
          vec2f( -1.0,  1.0),  // center, top
          vec2f( 1.0,  -1.0),  // right, center
          vec2f( 1.0,  1.0),  // right, top
        );

        var vsOutput: VertexShaderOutput;
        let xy = pos[vertexIndex];
        vsOutput.position = vec4f(xy, 0.0, 1.0);
        vsOutput.texcoord = uv[vertexIndex];
        return vsOutput;
      }

      @group(0) @binding(0) var ourSampler: sampler;
      @group(0) @binding(1) var ourTexture: texture_2d<f32>;

      @fragment fn fs(fsInput: VertexShaderOutput) -> @location(0) vec4f {
          let color = textureSample(ourTexture, ourSampler, fsInput.texcoord);
          return color;
      }
`;

export class GraphicsPipeline {
    device: GPUDevice;
    pipeline: GPURenderPipeline | undefined;
    sampler: GPUSampler | undefined;
    pipelineLayout: GPUPipelineLayout | undefined;
    inputTexture: GPUTexture | undefined;
    bindGroup: GPUBindGroup | undefined;

    constructor(device: GPUDevice) {
        this.device = device;
    }

    createGraphicsPipelineLayout() {
        // Passthrough shader will need an input texture to be displayed on the screen
        const bindGroupLayoutDescriptor: GPUBindGroupLayoutDescriptor = {
            label: 'pass through pipeline bind group layout',
            entries: [
                { binding: 0, visibility: GPUShaderStage.FRAGMENT, sampler: {} },
                { binding: 1, visibility: GPUShaderStage.FRAGMENT, texture: { sampleType: 'float' } },
            ],
        };

        const bindGroupLayout = this.device.createBindGroupLayout(bindGroupLayoutDescriptor);
        const layout = this.device.createPipelineLayout({ bindGroupLayouts: [bindGroupLayout] });
        this.pipelineLayout = layout;
    }

    createPipeline(shaderModule: GPUShaderModule, inputTexture: GPUTexture) {
        this.createGraphicsPipelineLayout();

        if (this.pipelineLayout == undefined) throw new Error("Pipeline layout not available");

        const pipeline = this.device.createRenderPipeline({
            label: 'pass through pipeline',
            layout: this.pipelineLayout,
            vertex:
            {
                module: shaderModule,
                entryPoint: "vs",
            },
            fragment:
            {
                module: shaderModule,
                entryPoint: "fs",
                targets: [{ format: navigator.gpu.getPreferredCanvasFormat(), }],
            },
        });
        this.pipeline = pipeline;

        this.sampler = this.device.createSampler();
        this.inputTexture = inputTexture;
        this.createBindGroup();
    }

    createBindGroup() {
        if (this.pipeline == undefined) throw new Error("Pipeline not created yet");
        if (this.sampler == undefined) throw new Error("Sampler not created yet");
        if (this.inputTexture == undefined) throw new Error("Input texture not created yet");
        const bindGroup = this.device.createBindGroup({
            label: 'pass through pipeline bind group',
            layout: this.pipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: this.sampler },
                { binding: 1, resource: this.inputTexture.createView() },
            ],
        });

        this.bindGroup = bindGroup;
    }

    createRenderPassDesc(view: GPUTextureView): GPURenderPassDescriptor {
        const attachment: GPURenderPassColorAttachment = {
            view,
            clearValue: [0.3, 0.3, 0.3, 1],
            loadOp: 'clear',
            storeOp: 'store',
        };
        const renderPassDescriptor = {
            label: 'pass through renderPass',
            colorAttachments: [
                attachment,
            ],
        };

        return renderPassDescriptor;
    }
};
